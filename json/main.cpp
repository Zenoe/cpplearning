#include "stdafx.h"

using json = nlohmann::json;


auto main(int argc, char *argv[]) -> int {
    (void)argc; (void)argv;   // Explicitly mark as unused if needed
    std::ifstream f("example.json");
    json data = json::parse(f);

    print(data["name"]);
    print(data["data"]);
    // print(data["data"]["users"]["id"]);

    return 0;
}
