// compile ok on vs2022/c++17
// read from sqlserver, save to csv
#include <windows.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <locale>
#include <codecvt>

#include <filesystem>
//#include <io.h>
//#include <fcntl.h>

void checkRet(SQLRETURN ret, SQLHANDLE handle, SQLSMALLINT type, const std::wstring& msg) {
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::wcerr << L"Error: " << msg << std::endl;
        SQLWCHAR sqlState[256], msgText[1024];
        SQLINTEGER nativeError;
        SQLSMALLINT textLength;
        SQLGetDiagRecW(type, handle, 1, sqlState, &nativeError, msgText, 1024, &textLength);
        std::wcerr << L"SQLState: " << sqlState << L", Msg: " << msgText << std::endl;
        exit(1);
    }
}

int main() {
    SQLHENV henv = nullptr;
    SQLHDBC hdbc = nullptr;
    SQLHSTMT hstmt = nullptr;
    SQLRETURN ret;

    // Init ODBC environment
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    checkRet(ret, henv, SQL_HANDLE_ENV, L"Alloc ENV");

    ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
    checkRet(ret, henv, SQL_HANDLE_ENV, L"Set Env Attr");

    // Init connection
    ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
    checkRet(ret, hdbc, SQL_HANDLE_DBC, L"Alloc DBC");

    std::wstring connStr =
        L"DRIVER={ODBC Driver 17 for SQL Server};"
        L"SERVER=172.29.106.90,1433;"
        L"DATABASE=SWProDebug;"
        L"UID=sa;"
        L"PWD=·1234567890-=autotest;"
        L"Encrypt=no;"
        L"TrustServerCertificate=yes;"
        L"Connection Timeout=15;";

    SQLWCHAR outConnStr[1024];
    SQLSMALLINT outConnStrLen;
    ret = SQLDriverConnectW(hdbc, NULL,
        (SQLWCHAR*)connStr.c_str(), SQL_NTS,
        outConnStr, sizeof(outConnStr) / sizeof(SQLWCHAR), &outConnStrLen,
        SQL_DRIVER_NOPROMPT);
    checkRet(ret, hdbc, SQL_HANDLE_DBC, L"Driver Connect");
    std::locale::global(std::locale(""));  //txt 编码是默认ansi
    //std::locale::global(std::locale(".65001"));// txt 编码是 utf8
    // Open files (wifstream/wofstream for Unicode)
    std::wifstream infile(L"data/PJ43CaseId.txt");
    const std::wstring outputfilename = L"output.csv";
    // Remove file if it exists
    std::filesystem::remove(outputfilename);
    std::wofstream outfile(outputfilename, std::ios::app);

    if (!infile || !outfile) {
        std::wcerr << L"File open error." << std::endl;
        return 1;
    }

	//_setmode(_fileno(stdout), _O_U16TEXT);
    // For each line in PJ43CaseId.txt
    std::wstring caseid, idstr;
    while (std::getline(infile, caseid)) {
        // Trim whitespace
        caseid.erase(0, caseid.find_first_not_of(L" \t\r\n"));
        caseid.erase(caseid.find_last_not_of(L" \t\r\n") + 1);

        //std::wcout << caseid << L"\n";
        //continue;
        if (caseid.empty())
            continue;
		// Prepare the SQL query (wide version!)
        std::wstringstream ss;
        ss << L"SELECT TOP 1 ID, CREATE_TIME FROM TEST_CASE_DATA_LOG "
            L"WHERE PROJECT_ID = '7AFAEA50-BCF2-F190-6CA3-3A1AC2ED5BC9' "
            //L"AND ANALYST_LOG LIKE N'%RG-三层路由域-BGP4+-ADMINDIST-93293-GN-001%' "
             L"AND ANALYST_LOG LIKE N'%" << caseid << L"%' "
            L"ORDER BY CREATE_TIME DESC";

        std::wstring sql = ss.str();
        //std::wcout << L"searching: " << ss.str() << std::endl;
        // Allocate statement
        if (hstmt) SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
        checkRet(ret, hstmt, SQL_HANDLE_STMT, L"Alloc STMT");

        // Execute (wide version)
        ret = SQLExecDirectW(hstmt, (SQLWCHAR*)sql.c_str(), SQL_NTS);
        if (ret != SQL_SUCCESS && ret != SQL_NO_DATA) {
            std::wcerr << L"SQL error: " << sql << " " << ret << std::endl;
            continue; // skip error
        }

        // Fetch result (wide)
        SQLWCHAR idbuf[128] = L"";
        SQLWCHAR ctimebuf[128] = L"";
        SQLLEN ind_id = 0, ind_ctime = 0;
        std::wstring idstr, ctimestr;
        idstr.clear();

        ret = SQLFetch(hstmt);
        if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
            // Get first column: ID
            ret = SQLGetData(hstmt, 1, SQL_C_WCHAR, idbuf, sizeof(idbuf), &ind_id);
            if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO && ind_id != SQL_NULL_DATA) {
                idstr = idbuf;
            }
            else {
                idstr.clear();
            }
            // Get second column: CREATE_TIME
            ret = SQLGetData(hstmt, 2, SQL_C_WCHAR, ctimebuf, sizeof(ctimebuf), &ind_ctime);
            if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO && ind_ctime != SQL_NULL_DATA) {
                ctimestr = ctimebuf;
            }
            else {
                ctimestr.clear();
            }
        }
        else {
            idstr.clear();
            ctimestr.clear();
        }
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        hstmt = nullptr;
        std::wstring urlstr = L"http://ats.ruijie.net/LogView/LogView?testcaseID=";
        urlstr += idstr;
        std::wcout << L"result: " << idstr.c_str() << L" " << ctimestr.c_str() << std::endl;

        // Write to CSV (quoted, wide)
        outfile << L"\"" << caseid << L"\",\"" << urlstr << L"\",\"" << ctimestr << L"\"\n";
    }

    // Cleanup
    if (hstmt) SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    if (hdbc) SQLDisconnect(hdbc);
    if (hdbc) SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    if (henv) SQLFreeHandle(SQL_HANDLE_ENV, henv);

    return 0;
}
