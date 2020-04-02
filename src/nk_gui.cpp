#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_IMPLEMENTATION
#define NK_GDI_IMPLEMENTATION
#include "nk_gui.h"

#include "nuklear/nuklear.h"
#include "nuklear/nuklear_gdi.h"
#include "window.h"
#include "filehandling.h"
#include <codecvt>
#include <locale>
#include <cstring>

GdiFont* font;
struct nk_context *ctx;
unsigned int windowWidth;
unsigned int windowHeight;

// was a file loaded?
bool fileOpen=false;
// deck number chosen through the combobox
int selectedDeck;
// series names displayed in the series combobox
std::vector<std::string> seriesStrings;
// informative string displayed in the log at the bottom
std::string infoString;
// strings edited by the textboxes
std::string editStringDeckNameID;
std::string editStringDeckName;

void SetupGui(WINDOW_DATA &windowData,unsigned int initialWindowWidth, unsigned int initialWindowHeight){
    font = nk_gdifont_create("Segoe UI", 18);
    windowWidth=initialWindowWidth;
    windowHeight=initialWindowHeight;
    ctx = nk_gdi_init(font, windowData.dc, initialWindowWidth, initialWindowHeight);
    
    selectedDeck=0;
    seriesStrings.push_back("DM");
    seriesStrings.push_back("GX");
    seriesStrings.push_back("5DS");
    seriesStrings.push_back("ZEXAL");
    seriesStrings.push_back("ARC-V");
    seriesStrings.push_back("VRAINS");
    seriesStrings.push_back("NONE");
    
    infoString="No file loaded. Open a deckdata_X.bin file to begin.";
    
}

void HandleInput(WINDOW_DATA &windowData){
        MSG msg;
        nk_input_begin(ctx);
        if (windowData.needs_refresh == 0) {
            if (GetMessageW(&msg, NULL, 0, 0) <= 0)
                windowData.running = 0;
            else {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
            windowData.needs_refresh = 1;
        } else windowData.needs_refresh = 0;

        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)
                windowData.running = 0;
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            windowData.needs_refresh = 1;
        }
        nk_input_end(ctx);
        
}

int HandleEvent(const EVENT_DATA &eventData){
    switch (eventData.msg)
    {
	case WM_SIZE:
        windowWidth=LOWORD(eventData.lparam);
        windowHeight=HIWORD(eventData.lparam);
		return nk_gdi_handle_event(eventData.wnd, eventData.msg, eventData.wparam, eventData.lparam);
    default:
        return nk_gdi_handle_event(eventData.wnd, eventData.msg, eventData.wparam, eventData.lparam);
    }
    
}

void HandleGui(FILE_DATA &fileData){
    if (nk_begin(ctx, "Demo", nk_rect(0, 0, windowWidth, windowHeight),
        0))
    {
        nk_layout_row_static(ctx, 0, 100, 2);
        if (nk_menu_begin_label(ctx,"FILE",NK_TEXT_LEFT,nk_vec2(100, 100))){
            nk_layout_row_dynamic(ctx, 0, 1);
            if(nk_menu_item_label(ctx, "OPEN", NK_TEXT_LEFT)){
                std::string filename=OpenFilename("YGO LOTD LE Deck Data Files (*.bin)\0*.*\0");
                if (!filename.empty()){
                    bool success=ReadFile(filename,fileData);
                    if (success){
                        fileOpen=true;
                        selectedDeck=0;
                        UpdateTextEdits(selectedDeck,fileData);
                        infoString=std::to_string(fileData.deckCount)+" decks loaded from file "+filename;
                    }
                    else{
                        infoString="Either the file could not be opened, or there was a read error.";
                    }
                }
            }
            if(nk_menu_item_label(ctx, "SAVE", NK_TEXT_LEFT) && fileOpen){
                std::string filename=SaveFilename("YGO LOTD LE Deck Data Files (*.bin)\0*.*\0");
                if (!filename.empty()){
                    UpdateStrings(selectedDeck,fileData);
                    bool success=SaveFile(filename,fileData);
                    if (success){
                        infoString="Successfully saved data to "+filename;
                    }else{
                        infoString="Either the file could not be created, or there was a write error.";
                    }

                }
            }
            nk_menu_end(ctx);
        }
        if (nk_menu_begin_label(ctx,"EDIT",NK_TEXT_LEFT,nk_vec2(100, 100))){
            nk_layout_row_dynamic(ctx, 0, 1);
            if(nk_menu_item_label(ctx, "Add Deck Slot", NK_TEXT_LEFT) && fileOpen){
                UpdateStrings(selectedDeck,fileData);
                fileData.deckCount++;
                selectedDeck=fileData.deckCount-1;
                fileData.field1.push_back(fileData.field1[selectedDeck-1]+1);
                fileData.field2.push_back(fileData.field1[selectedDeck]);
                fileData.field3.push_back(0xFFFFFFFF);
                fileData.field4.push_back(4007); // BEWD
                fileData.field5.push_back(1);
                fileData.field6.push_back(0xFFFFFFFF);
                fileData.string1.push_back("Deck"+std::to_string(fileData.field1[selectedDeck]));
                fileData.string2.push_back("Deck "+std::to_string(fileData.field1[selectedDeck]));
                fileData.string3.push_back(u"");
                fileData.string4.push_back(u"");
                UpdateTextEdits(selectedDeck,fileData);
            }
            nk_menu_end(ctx);
        }
        
        float ratio[]={0.2,0.6,0.2};
        nk_layout_row(ctx, NK_DYNAMIC, 0, 3, ratio);
        
        if (nk_button_label(ctx, "Prev") && fileOpen){
            if (selectedDeck>0){
                UpdateStrings(selectedDeck,fileData);
                selectedDeck--;
                UpdateTextEdits(selectedDeck,fileData);
            }
        }

        int comboWasClicked;
        std::string comboLabel;
        if (fileOpen)
            comboLabel=std::to_string(fileData.field1[selectedDeck])+" - "+fileData.string1[selectedDeck];
        if(nk_combo_begin_label(ctx, comboLabel.c_str(),nk_vec2(219,190),&comboWasClicked)){
            if (comboWasClicked){
                nk_popup_set_scroll(ctx,0,selectedDeck*(ctx->current->layout->row.min_height+ctx->style.window.spacing.y));
            }
            if (fileOpen){
                nk_layout_row_dynamic(ctx, 0, 1);
                for (Long i=0; i< fileData.deckCount; i++){
                    int selected=nk_false;
                    if (nk_selectable_label(ctx,(std::to_string(fileData.field1[i])+" - "+fileData.string1[i]).c_str(),NK_TEXT_LEFT,&selected)){
                        if (selectedDeck!=i){
                            UpdateStrings(selectedDeck,fileData);
                            selectedDeck=i;
                            UpdateTextEdits(selectedDeck,fileData);
                        }
                        nk_combo_close(ctx);
                    }
                }
            }
            nk_combo_end(ctx);
        }

        if (nk_button_label(ctx, "Next") && fileOpen){
            if (selectedDeck+1<fileData.deckCount){
                UpdateStrings(selectedDeck,fileData);
                selectedDeck++;
                UpdateTextEdits(selectedDeck,fileData);
            }
        }
        nk_layout_row_dynamic(ctx, 275, 3);

        if(nk_group_begin_titled(ctx, "group_fields", "Fields", NK_WINDOW_TITLE|NK_WINDOW_BORDER)){
            nk_layout_row_dynamic(ctx, 0, 1);
            nk_label(ctx,"Deck ID",NK_TEXT_LEFT);
            nk_label(ctx,"Deck ID ?",NK_TEXT_LEFT);
            nk_label(ctx,"Series ID",NK_TEXT_LEFT);
            nk_label(ctx,"Signature Card ID",NK_TEXT_LEFT);
            nk_label(ctx,"Character ID",NK_TEXT_LEFT);
            nk_label(ctx,"DLC Flag?",NK_TEXT_LEFT);
            nk_group_end(ctx);
        }
        
        if(nk_group_begin_titled(ctx, "group_hex", "Hex", NK_WINDOW_TITLE|NK_WINDOW_BORDER)){
            if (fileOpen){
                nk_layout_row_dynamic(ctx, 0, 1);
                nk_label(ctx,IntToHexString(fileData.field1[selectedDeck]).c_str(),NK_TEXT_LEFT);
                nk_label(ctx,IntToHexString(fileData.field2[selectedDeck]).c_str(),NK_TEXT_LEFT);
                nk_label(ctx,IntToHexString(fileData.field3[selectedDeck]).c_str(),NK_TEXT_LEFT);
                nk_label(ctx,IntToHexString(fileData.field4[selectedDeck]).c_str(),NK_TEXT_LEFT);
                nk_label(ctx,IntToHexString(fileData.field5[selectedDeck]).c_str(),NK_TEXT_LEFT);
                nk_label(ctx,IntToHexString(fileData.field6[selectedDeck]).c_str(),NK_TEXT_LEFT);
            }
            nk_group_end(ctx);
        }

        if(nk_group_begin_titled(ctx, "group_edit", "Edit", NK_WINDOW_TITLE|NK_WINDOW_BORDER)){
            if (fileOpen){
                nk_layout_row_dynamic(ctx, 0, 1);
                nk_label(ctx,std::to_string((int)(fileData.field1[selectedDeck])).c_str(),NK_TEXT_LEFT);
                nk_label(ctx,std::to_string((int)(fileData.field2[selectedDeck])).c_str(),NK_TEXT_LEFT);
                int seriesSelection=fileData.field3[selectedDeck];
                if (seriesSelection<0 || seriesSelection >=seriesStrings.size())
                    seriesSelection=seriesStrings.size()-1;
                if(nk_combo_begin_label(ctx, seriesStrings[seriesSelection].c_str(),nk_vec2(102,265))){
                    nk_layout_row_dynamic(ctx, 0, 1);
                    for (Long i=0; i< seriesStrings.size(); i++){
                        int selected=nk_false;
                        if (nk_selectable_label(ctx,seriesStrings[i].c_str(),NK_TEXT_LEFT,&selected)){
                            seriesSelection=i;
                            if (seriesSelection>=seriesStrings.size()-1)
                                fileData.field3[selectedDeck]=0xFFFFFFFF;
                            else
                                fileData.field3[selectedDeck]=i;
                            nk_combo_close(ctx);
                            
                        }
                    }
                    nk_combo_end(ctx);
                }
                
                nk_property_int(ctx,"#",3900,reinterpret_cast<int*>(&(fileData.field4[selectedDeck])),14968,1,1.0);
                nk_property_int(ctx,"#",1,reinterpret_cast<int*>(&(fileData.field5[selectedDeck])),INT_MAX,1,1.0);

                nk_label(ctx,std::to_string((int)(fileData.field6[selectedDeck])).c_str(),NK_TEXT_LEFT);
            }
            nk_group_end(ctx);
        }

        nk_layout_row_dynamic(ctx, 200, 1);
        if(nk_group_begin_titled(ctx, "group_strings", "Strings", NK_WINDOW_TITLE|NK_WINDOW_BORDER)){
            if (fileOpen){
                float ratio[]={0.2,0.8};
                nk_layout_row(ctx, NK_DYNAMIC, 0, 2, ratio);
                nk_label(ctx,"ID String: ",NK_TEXT_LEFT);
                nk_edit_string_zero_terminated(ctx,NK_EDIT_FIELD|NK_EDIT_SELECTABLE ,const_cast<char*>(editStringDeckNameID.c_str()),52,0);
                nk_label(ctx,"Name: ",NK_TEXT_LEFT);
                nk_edit_string_zero_terminated(ctx,NK_EDIT_FIELD|NK_EDIT_SELECTABLE ,const_cast<char*>(editStringDeckName.c_str()),52,0);
                std::string u8_conv = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(fileData.string3[selectedDeck]);
                nk_label(ctx,"???: ",NK_TEXT_LEFT);
                nk_label(ctx,u8_conv.c_str(),NK_TEXT_LEFT);
                u8_conv = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(fileData.string4[selectedDeck]);
                nk_label(ctx,"???: ",NK_TEXT_LEFT);
                nk_label(ctx,u8_conv.c_str(),NK_TEXT_LEFT);
            }
            nk_group_end(ctx);
        }
        nk_layout_row_dynamic(ctx,0,1);
        nk_edit_string_zero_terminated(ctx,NK_EDIT_INACTIVE,const_cast<char*>(("INFO: "+infoString).c_str()),INT_MAX,nk_filter_default);
    }
    nk_end(ctx);
}

void UpdateStrings(int idx, FILE_DATA &fileData){
    fileData.string1[idx]=editStringDeckNameID;
    fileData.string1[idx].resize(strlen(&editStringDeckNameID[0]));
    fileData.string2[idx]=editStringDeckName;
    fileData.string2[idx].resize(strlen(&editStringDeckName[0]));
}

void UpdateTextEdits(int idx, FILE_DATA &fileData){
    editStringDeckNameID=fileData.string1[idx];
    editStringDeckNameID.resize(52);
    editStringDeckName=fileData.string2[idx];
    editStringDeckName.resize(52);
}


void RenderGui(){
    nk_gdi_render(nk_rgb(30,30,30));
}

void CleanupGui(){
    nk_gdifont_del(font);
}

void UpdateWindowSize(unsigned int width, unsigned int height){
    windowWidth=width;
    windowHeight=height;
}