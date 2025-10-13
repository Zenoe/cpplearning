#ifndef CLIPITEM_H_
#define CLIPITEM_H_
#include <string>
#include <vector>

struct ClipItem {
    int id = 0;
    std::wstring content;
    std::wstring timestamp;
    /* ClipItem() = default; */
};

struct DisplayClipItem : ClipItem {
    std::vector<bool> highlight_mask;
    /* DisplayClipItem() = default; // This now works! */
    DisplayClipItem(const ClipItem& clip, std::vector<bool> mask)
        : ClipItem(clip), highlight_mask(std::move(mask)) {}
};


#endif // CLIPITEM_H_
