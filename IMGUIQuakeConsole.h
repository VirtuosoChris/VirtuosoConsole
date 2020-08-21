//
//  ConsoleWidget.h
//
//  Created by VirtuosoChris on 8/17/20.
//  Forked from Example Console code in IMGUI samples
//  https://github.com/ocornut/imgui/blob/master/imgui_demo.cpp

#ifndef ConsoleWidget_h
#define ConsoleWidget_h

#include "QuakeStyleConsole.h"

struct ConsoleBuf : public std::streambuf
{
    std::vector<std::string> items;
    bool parsingANSICode = false;

    struct membuf: std::streambuf
    {
        membuf(const char* begin, const char* end)
        {
             this->setg((char*)begin, (char*)begin, (char*)end);
        }
    };
    
    ConsoleBuf()
    {
        items.push_back("");
    }
    
    std::string& curStr()
    {
        int idx = items.size()-1;
        return items[idx];
    }

    int overflow(int c)
    {
        if (c != EOF)
        {
            switch (c)
            {
                case '\n':
                {
                    items.push_back("");
                    break;
                }
                case 'm':
                {
                    curStr() +=(char)c;
                    if (parsingANSICode)
                    {
                        items.push_back("");
                        parsingANSICode = false;
                    }

                    break;
                }
                default:
                {
                     curStr() +=(char)c;
                }
            }
        }

        return c;
    }

    std::streamsize xsputn ( const char * s, std::streamsize n )
    {
        membuf mb(s, s + n);
        
        std::istream ib(&mb);

        while (!ib.eof())
        {
            std::string ln;
            std::getline(ib, ln);
            curStr()+=ln;

            if (!ib.eof())
            {
                items.push_back("");
            }
        }
        
        return n;
    }
};


#include <unordered_set>
class MultiStreamBuf: public  std::streambuf
{
    public:

        std::unordered_set<std::ostream*> streams;
       
        MultiStreamBuf()
        {
        }

        int overflow(int in)
        {
            char c = in;///\todo check for eof, etc?
            for (std::ostream* str : streams )
            {
                (*str) << c;
            }
            return 1;
        }

        std::streamsize xsputn ( const char * s, std::streamsize n )
        {
            std::streamsize ssz=0;

            for (std::ostream* str : streams )
            {
               ssz = str->rdbuf()->sputn(s, n);
            }

            return ssz;
        }
};

class MultiStream : public std::ostream
{
    MultiStreamBuf buf;
public:
    MultiStream() : std::ostream(&buf)
    {
        
    }
    
    void addStream(std::ostream& str)
    {
       buf.streams.insert(&str);
    }
};


struct IMGUIQuakeConsole : public Virtuoso::QuakeStyleConsole, public MultiStream//, public std::ostream
{
    ConsoleBuf            strb;
    std::ostream          consoleStream;
    char                  InputBuf[256];
    int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
    ImGuiTextFilter       Filter;
    bool                  AutoScroll;
    bool                  ScrollToBottom;
    
    IMGUIQuakeConsole() : consoleStream(&strb)
    {
        addStream(consoleStream);
        
        ClearLog();
        memset(InputBuf, 0, sizeof(InputBuf));
        HistoryPos = -1;
       
        AutoScroll = true;
        ScrollToBottom = false;
        
        bindMemberCommand("Clear", *this, &IMGUIQuakeConsole::ClearLog, "Clear the console");
    }
    
    ~IMGUIQuakeConsole()
    {
    }

    // Portable helpers
    static int   Stricmp(const char* s1, const char* s2)         { int d; while ((d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; } return d; }
    static int   Strnicmp(const char* s1, const char* s2, int n) { int d = 0; while (n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; n--; } return d; }
    static char* Strdup(const char* s)                           { size_t len = strlen(s) + 1; void* buf = malloc(len); IM_ASSERT(buf); return (char*)memcpy(buf, (const void*)s, len); }
    static void  Strtrim(char* s)                                { char* str_end = s + strlen(s); while (str_end > s && str_end[-1] == ' ') str_end--; *str_end = 0; }

    void ClearLog()
    {
        strb.items.clear();
        strb.items.push_back("");
    }
    
    void optionsMenu()
    {
        ImGui::Checkbox("Auto-scroll", &AutoScroll);
    }

    void Draw(const char* title, bool* p_open)
    {
        ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(title, p_open))
        {
            ImGui::End();
            return;
        }

        // As a specific feature guaranteed by the library, after calling Begin() the last Item represent the title bar.
        // So e.g. IsItemHovered() will return true when hovering the title bar.
        // Here we create a context menu only available from the title bar.
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Close Console"))
                *p_open = false;
            ImGui::EndPopup();
        }

        ImGui::TextWrapped("Enter 'HELP' for help, press TAB to use text completion.");

        // TODO: display items starting from the bottom
        
        if (ImGui::SmallButton("Clear")){ ClearLog(); } ImGui::SameLine();
        
        bool copy_to_clipboard = ImGui::SmallButton("Copy");

        ImGui::Separator();

        // Options menu
        if (ImGui::BeginPopup("Options"))
        {
            optionsMenu();
            ImGui::EndPopup();
        }

        // Options, Filter
        if (ImGui::Button("Options"))
            ImGui::OpenPopup("Options");
        ImGui::SameLine();
        Filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
        ImGui::Separator();

        // Reserve enough left-over height for 1 separator + 1 input text
        const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);
        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::Selectable("Clear")) ClearLog();
            ImGui::EndPopup();
        }

        // Display every line as a separate entry so we can change their color or add custom widgets.
        // If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
        // NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping
        // to only process visible items. The clipper will automatically measure the height of your first item and then
        // "seek" to display only items in the visible area.
        // To use the clipper we can replace your standard loop:
        //      for (int i = 0; i < Items.Size; i++)
        //   With:
        //      ImGuiListClipper clipper(Items.Size);
        //      while (clipper.Step())
        //         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
        // - That your items are evenly spaced (same height)
        // - That you have cheap random access to your elements (you can access them given their index,
        //   without processing all the ones before)
        // You cannot this code as-is if a filter is active because it breaks the 'cheap random-access' property.
        // We would need random-access on the post-filtered list.
        // A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices
        // or offsets of items that passed the filtering test, recomputing this array when user changes the filter,
        // and appending newly elements as they are inserted. This is left as a task to the user until we can manage
        // to improve this example code!
        // If your items are of variable height:
        // - Split them into same height items would be simpler and facilitate random-seeking into your list.
        // - Consider using manual call to IsRectVisible() and skipping extraneous decoration from your items.
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
        if (copy_to_clipboard)
            ImGui::LogToClipboard();
        for (int i = 0; i < strb.items.size(); i++)
        {
            const char* item = strb.items[i].c_str();
            if (!Filter.PassFilter(item))
                continue;
            
            // Normally you would store more information in your item than just a string.
            // (e.g. make Items[] an array of structure, store color/type etc.)
            ImVec4 color;
            bool has_color = false;
            if (strstr(item, "[error]"))
            {
                color = ImVec4(2.0f, 0.2f, 0.2f, 1.0f); has_color = true;
            }
            else if (strncmp(item, "# ", 2) == 0)
            {
                color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f); has_color = true;
            }
            if (strstr(item, "[warning]"))
            {
                color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); has_color = true;
            }
            if (strstr(item, "> "))
            {
                 color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f); has_color = true;
            }
            
            if (has_color)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, color);
            }
            
            // *** actually do the text **
            ImGui::TextUnformatted(item);
            
            if (has_color)
            {
                ImGui::PopStyleColor();
            }
        }
        if (copy_to_clipboard)
            ImGui::LogFinish();

        if (ScrollToBottom || (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
            ImGui::SetScrollHereY(1.0f);
        ScrollToBottom = false;

        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::Separator();

        // Command-line
        bool reclaim_focus = false;
        ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
        if (ImGui::InputText("Input", InputBuf, IM_ARRAYSIZE(InputBuf), input_text_flags, &TextEditCallbackStub, (void*)this))
        {
            char* s = InputBuf;
            Strtrim(s);
            if (s[0])
                ExecCommand(s);
            strcpy(s, "");
            reclaim_focus = true;
        }

        // Auto-focus on window apparition
        ImGui::SetItemDefaultFocus();
        if (reclaim_focus)
            ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

        ImGui::End();
    }

    void ExecCommand(const char* command_line)
    {
        // Insert into history. First find match and delete it so it can be pushed to the back.
        // This isn't trying to be smart or optimal.
        HistoryPos = -1;
        
        commandExecute(command_line, *this);

        // On command input, we scroll to bottom even if AutoScroll==false
        ScrollToBottom = true;
    }

    // In C++11 you'd be better off using lambdas for this sort of forwarding callbacks
    static int TextEditCallbackStub(ImGuiInputTextCallbackData* data)
    {
        IMGUIQuakeConsole* console = (IMGUIQuakeConsole*)data->UserData;
        return console->TextEditCallback(data);
    }

    int TextEditCallback(ImGuiInputTextCallbackData* data)
    {
        switch (data->EventFlag)
        {
            case ImGuiInputTextFlags_CallbackCompletion:
            {
                // Example of TEXT COMPLETION

                // Locate beginning of current word
                const char* word_end = data->Buf + data->CursorPos;
                const char* word_start = word_end;
                while (word_start > data->Buf)
                {
                    const char c = word_start[-1];
                    if (c == ' ' || c == '\t' || c == ',' || c == ';')
                        break;
                    word_start--;
                }

                // Build a list of candidates
                ImVector<const char*> candidates;
                //for (int i = 0; i < Commands.Size; i++)
                for (auto it = commandTable.begin(); it != commandTable.end(); it++)
                {
                    const char* cmd = it->first.c_str();
                    if (Strnicmp(cmd, word_start, (int)(word_end - word_start)) == 0)
                    {
                        candidates.push_back(cmd);
                    }
                }
                
                // also check variables for matches
                for (auto it = cvarPrintFTable.begin(); it != cvarPrintFTable.end(); it++)
                {
                    const char* cmd = it->first.c_str();
                    if (Strnicmp(cmd, word_start, (int)(word_end - word_start)) == 0)
                    {
                        candidates.push_back(cmd);
                    }
                }

                if (candidates.Size == 0)
                {
                    // No match
                    //AddLog("No match for %.*s, , word_start);
                    (*this)<<"No match for ";
                    (*this)<<(int)(word_end - word_start);
                    (*this)<<' '<<word_start;
                    (*this)<<"!\n";
                }
                else if (candidates.Size == 1)
                {
                    // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing.
                    data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                    data->InsertChars(data->CursorPos, candidates[0]);
                    data->InsertChars(data->CursorPos, " ");
                }
                else
                {
                    // Multiple matches. Complete as much as we can..
                    // So inputing "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as matches.
                    int match_len = (int)(word_end - word_start);
                    for (;;)
                    {
                        int c = 0;
                        bool all_candidates_matches = true;
                        for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
                            if (i == 0)
                                c = toupper(candidates[i][match_len]);
                            else if (c == 0 || c != toupper(candidates[i][match_len]))
                                all_candidates_matches = false;
                        if (!all_candidates_matches)
                            break;
                        match_len++;
                    }

                    if (match_len > 0)
                    {
                        data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                        data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
                    }

                    // List matches
                    (*this)<<"Possible matches:\n";
                    for (int i = 0; i < candidates.Size; i++)
                        (*this)<<"- "<<candidates[i]<<'\n';
                }

                break;
            }
            case ImGuiInputTextFlags_CallbackHistory:
            {
                // Example of HISTORY
                const int prev_history_pos = HistoryPos;
                if (data->EventKey == ImGuiKey_UpArrow)
                {
                    if (HistoryPos == -1)
                        HistoryPos = history_buffer.size() - 1;
                    else if (HistoryPos > 0)
                        HistoryPos--;
                }
                else if (data->EventKey == ImGuiKey_DownArrow)
                {
                    if (HistoryPos != -1)
                        if (++HistoryPos >= history_buffer.size())
                            HistoryPos = -1;
                }

                // A better implementation would preserve the data on the current input line along with cursor position.
                if (prev_history_pos != HistoryPos)
                {
                    const char* history_str = (HistoryPos >= 0) ? history_buffer[HistoryPos].c_str() : "";
                    data->DeleteChars(0, data->BufTextLen);
                    data->InsertChars(0, history_str);
                }
            }
        }
        return 0;
    }
};

#endif /* ConsoleWidget_h */
