//
//  IMGUIQuakeConsole.h
//
//  Created by VirtuosoChris on 8/17/20.
//  Forked from Example Console code in IMGUI samples
//  https://github.com/ocornut/imgui/blob/master/imgui_demo.cpp

#ifndef ConsoleWidget_h
#define ConsoleWidget_h

#include <regex>
#include <unordered_set>
#include <algorithm>

//dependencies
#include <imgui.h>
#include "QuakeStyleConsole.h"

const ImVec4 COMMENT_COLOR = ImVec4(1.0f, 0.8f, 0.6f, 1.0f);
const ImVec4 ERROR_COLOR = ImVec4(2.0f, 0.2f, 0.2f, 1.0f);
const ImVec4 WARNING_COLOR = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);

#define RED_BKGRND_COLOR        IM_COL32(255,0,0,255);
#define GREEN_BKGRND_COLOR      IM_COL32(0,255,0,255);
#define YELLOW_BKGRND_COLOR     IM_COL32(255,255,0,255);
#define BLUE_BKGRND_COLOR       IM_COL32(0,0,255,255);
#define MAGENTA_BKGRND_COLOR    IM_COL32(255,0,255,255);
#define CYAN_BKGRND_COLOR       IM_COL32(0,255,255,255);
#define WHITE_BKGRND_COLOR      IM_COL32(255,255,255,255);

// Portable helpers

static int   Strnicmp(const char* s1, const char* s2, int n) { int d = 0; while (n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; n--; } return d; }

static void  Strtrim(char* s)                                { char* str_end = s + strlen(s); while (str_end > s && str_end[-1] == ' ') str_end--; *str_end = 0; }


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

struct ColorTokenizer;
void DO_NOTHING(ColorTokenizer& tok, const std::string& str){}

struct Rule
{
    std::regex rule;
    ImVec4 color;
    bool hasColor = false;
    ImU32 backgroundColor = 0;
    bool hasBackgroundColor = false;
    bool changeDefaultColors = false;
    bool showToken = true;
    std::function<void (ColorTokenizer&, const std::string&)> action = DO_NOTHING;
};

typedef std::vector<Rule> RuleSet;


enum AnsiColorCode
{
    ANSI_RESET          = 0,
    ANSI_BRIGHT_TEXT    = 1,
    
    ANSI_BLACK          = 30,
    ANSI_RED            = 31,
    ANSI_GREEN          = 32,
    ANSI_YELLOW         = 33,
    ANSI_BLUE           = 34,
    ANSI_MAGENTA        = 35,
    ANSI_CYAN           = 36,
    ANSI_WHITE          = 37,
    
    ANSI_BLACK_BKGRND   = 40,
    ANSI_RED_BKGRND     = 41,
    ANSI_GREEN_BKGRND   = 42,
    ANSI_YELLOW_BKGRND  = 43,
    ANSI_BLUE_BKGRND    = 44,
    ANSI_MAGENTA_BKGRND = 45,
    ANSI_CYAN_BKGRND    = 46,
    ANSI_WHITE_BKGRND   = 47,
};


struct ColorTokenizer;
void processANSICode(ColorTokenizer& tok, int code, bool& brightText, AnsiColorCode& textCode);
void handleANSIString(ColorTokenizer& tok, const std::string& str);

// reference : http://www.cplusplus.com/reference/regex/ECMAScript/
void makeDefaultRules(RuleSet& rules)
{
    
    {
        Rule r;
        r.rule = std::regex("\\[error\\].*");
        r.color = ERROR_COLOR;
        r.hasColor = true;
        rules.push_back(r);
    }
    
    {
        Rule r;
        r.rule = std::regex("\\[warning\\].*");
        r.color = WARNING_COLOR;
        r.hasColor = true;
        rules.push_back(r);
    }
    
    { // pound at beginning of line treated as comment until end of line
        Rule r;
        r.rule = std::regex("(\n|^)#.*");
        r.color = COMMENT_COLOR;
        r.hasColor = true;
        rules.push_back(r);
    }
    
    { // c++ style // comments, anywhere in the line til the end of the line
        Rule r;
        r.rule = std::regex("\/\/.*");
        r.color = COMMENT_COLOR;
        r.hasColor = true;
        rules.push_back(r);
    }
    
    { // Start line with "> " for echoing output, captures til end of line
        Rule r;
        r.rule = std::regex("(\n|^)> .*");
        r.color = COMMENT_COLOR;
        r.hasColor = true;
        rules.push_back(r);
    }
    
    {
        Rule ansiColorRule;
        ansiColorRule.rule = std::regex("\033\[[0-9]+(;[0-9]+)*m");
        ansiColorRule.showToken = false;
        ansiColorRule.action = handleANSIString;
        rules.push_back(ansiColorRule);
    }
}


ImU32 getANSIBackgroundColor(AnsiColorCode code)
{
    switch (code)
    {
        case ANSI_RESET:
            return 0;
        case ANSI_BLACK_BKGRND:
            return 0;
        case ANSI_RED_BKGRND:
            return RED_BKGRND_COLOR;
        case ANSI_GREEN_BKGRND:
            return GREEN_BKGRND_COLOR;
        case ANSI_YELLOW_BKGRND:
            return YELLOW_BKGRND_COLOR;
        case ANSI_BLUE_BKGRND:
            return BLUE_BKGRND_COLOR;
        case ANSI_MAGENTA_BKGRND:
            return MAGENTA_BKGRND_COLOR;
        case ANSI_CYAN_BKGRND:
            return CYAN_BKGRND_COLOR;
        case ANSI_WHITE_BKGRND:
            return WHITE_BKGRND_COLOR;
        default:
            return 0;
    }
}

ImVec4 getAnsiTextColor(AnsiColorCode code)
{
    switch (code)
    {
        case ANSI_RESET:
            return ImVec4(0.750,0.750,0.750,1.0);
        case ANSI_BLACK:
            return ImVec4(0.0,0.0,0.0,1.0);
        case ANSI_RED:
            return ImVec4(0.75,0.0,0.0,1.0);
        case ANSI_GREEN:
            return ImVec4(0.0,0.750,0.0,1.0);
        case ANSI_YELLOW:
            return ImVec4(0.750,0.750,0.0,1.0);
        case ANSI_BLUE:
           return ImVec4(0.0,0.0,0.750,1.0);;
        case ANSI_MAGENTA:
           return ImVec4(0.750,0.0,0.750,1.0);;
        case ANSI_CYAN:
           return ImVec4(0.0,0.750,0.750,1.0);;
        case ANSI_WHITE:
           return ImVec4(0.750,0.750,0.750,1.0);;
        default:
            return ImVec4(0.0,0.0,0.0,1.0);
    }
}


ImVec4 getAnsiTextColorBright(AnsiColorCode code)
{
    switch (code)
    {
        case ANSI_RESET:
            return ImVec4(1.0,1.0,1.0,1.0);
        case ANSI_BLACK:
            return ImVec4(0.0,0.0,0.0,1.0);
        case ANSI_RED:
            return ImVec4(1.0,0.0,0.0,1.0);
        case ANSI_GREEN:
            return ImVec4(0.0,1.0,0.0,1.0);
        case ANSI_YELLOW:
            return ImVec4(1.0,1.0,0.0,1.0);
        case ANSI_BLUE:
           return ImVec4(0.0,0.0,1.0,1.0);;
        case ANSI_MAGENTA:
           return ImVec4(1.0,0.0,1.0,1.0);;
        case ANSI_CYAN:
           return ImVec4(0.0,1.0,1.0,1.0);;
        case ANSI_WHITE:
           return ImVec4(1.0,1.0,1.0,1.0);;
        default:
            return ImVec4(0.0,0.0,0.0,1.0);
    }
}

const std::string TEXT_COLOR_RESET              =   "\u001b[0m";
const std::string TEXT_COLOR_BLACK              =   "\u001b[30m";
const std::string TEXT_COLOR_RED                =   "\u001b[31m";
const std::string TEXT_COLOR_GREEN              =   "\u001b[32m";
const std::string TEXT_COLOR_YELLOW             =   "\u001b[33m";
const std::string TEXT_COLOR_BLUE               =   "\u001b[34m";
const std::string TEXT_COLOR_MAGENTA            =   "\u001b[35m";
const std::string TEXT_COLOR_CYAN               =   "\u001b[36m";
const std::string TEXT_COLOR_WHITE              =   "\u001b[37m";
const std::string TEXT_COLOR_BLACK_BRIGHT       =   "\u001b[30;1m";
const std::string TEXT_COLOR_RED_BRIGHT         =   "\u001b[31;1m";
const std::string TEXT_COLOR_GREEN_BRIGHT       =   "\u001b[32;1m";
const std::string TEXT_COLOR_YELLOW_BRIGHT      =   "\u001b[33;1m";
const std::string TEXT_COLOR_BLUE_BRIGHT        =   "\u001b[34;1m";
const std::string TEXT_COLOR_MAGENTA_BRIGHT     =   "\u001b[35;1m";
const std::string TEXT_COLOR_CYAN_BRIGHT        =   "\u001b[36;1m";
const std::string TEXT_COLOR_WHITE_BRIGHT       =   "\u001b[37;1m";
const std::string TEXT_COLOR_BLACK_BKGRND       =   "\u001b[40m";
const std::string TEXT_COLOR_RED_BKGRND         =   "\u001b[41m";
const std::string TEXT_COLOR_GREEN_BKGRND       =   "\u001b[42m";
const std::string TEXT_COLOR_YELLOW_BKGRND      =   "\u001b[43m";
const std::string TEXT_COLOR_BLUE_BKGRND        =   "\u001b[44m";
const std::string TEXT_COLOR_MAGENTA_BKGRND     =   "\u001b[45m";
const std::string TEXT_COLOR_CYAN_BKGRND        =   "\u001b[46m";
const std::string TEXT_COLOR_WHITE_BKGRND       =   "\u001b[47m";

struct ColorTokenizer
{
    RuleSet rules;
    
    ImVec4 textColor = ImVec4(1.0,1.0,1.0,1.0);
    ImU32 backgroundColor = 0;
    AnsiColorCode lastColorToken = ANSI_WHITE;
    bool hasBackgroundColor = false;
    
    void defaultFormatting() // call at beginning of output stream
    {
        textColor = ImVec4(1.0,1.0,1.0,1.0);
        backgroundColor = 0;
        lastColorToken = ANSI_WHITE;
        hasBackgroundColor = false;
    }
    
    void matched(Rule& tok, const std::string& str)
    {
        if (tok.showToken)
        {
            if (tok.hasColor)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, tok.color);
            }
            
            ImGui::TextUnformatted(str.c_str());
            
            if (tok.hasColor)
            {
                ImGui::PopStyleColor();
            }
            
            ImGui::SameLine();
        }
        
        if (tok.changeDefaultColors)
        {
            if (tok.hasColor) textColor =  tok.color;
            if (tok.hasBackgroundColor)
            {
                backgroundColor = tok.backgroundColor;
            }
        }
        
        tok.action(*this, str);
    }
    
    void unmatched(const std::string& str)
    {
        if (hasBackgroundColor)
        {
            ImVec2 textSize =  ImGui::CalcTextSize(str.c_str());
            ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
            ImVec2 sum = ImVec2(textSize[0] + cursorScreenPos[0], textSize[1] + cursorScreenPos[1]);
            ImGui::GetWindowDrawList()->AddRectFilled(cursorScreenPos, sum, backgroundColor);
        }
        
        ImGui::TextColored(textColor, str.c_str());
        ImGui::SameLine();
    }
    
    void checkRules(const std::string& str)
    {
        struct SearchParams
        {
            std::smatch match;
            std::size_t searchStartedAt = 0;
            std::string result;
            std::string prefix;
            std::string suffix;
            bool hasResult = true;
            
            std::size_t offsetOfMatch()const
            {
                return match.prefix().length() + searchStartedAt;
            }
        };
        
        std::vector<SearchParams> matches;
        
        matches.resize(rules.size());
        
        int segmentStart = 0;
        int priorityMatch = -1;
        
        do
        {
            int lastPriorityMatch = priorityMatch;
            priorityMatch = -1;
            
            unsigned int startLocation = 0xffffffff;
            
            for (int i = 0; i < rules.size(); i++)
            {
                SearchParams& match = matches[i];
                
                if (!match.hasResult)continue;
                
                // update the next search result for the regex we chose, or for any regex whose latest result
                // is now behind the edge of the string we've consumed
                if ( (i == lastPriorityMatch) || (match.offsetOfMatch() < segmentStart) || (!segmentStart))
                {
                    match.hasResult = std::regex_search( (str.begin() + segmentStart), str.end()
                                                        , match.match, rules[i].rule);
                    
                    match.searchStartedAt = segmentStart;
                    
                    if (match.hasResult)
                    {
                        //std::cout << "\tpossible match "<<match.match.str()<<std::endl;
                        match.prefix = match.match.prefix();
                        match.suffix = match.match.suffix();
                        match.result = match.match.str();
                    }
                }
                
                // get the nearest for any
                if (match.hasResult && match.offsetOfMatch() < startLocation)
                {
                    priorityMatch = i;
                    std::size_t off = match.offsetOfMatch();
                    startLocation = off;
                }
            }
            
            if (priorityMatch == -1)
            {
                unmatched(str.substr(segmentStart, str.size() - segmentStart));
                
                return;
            }
            
            // handle first match
            
            const std::string prefix = matches[priorityMatch].prefix;
            const std::string matchStr = matches[priorityMatch].result;
            
            if (prefix.length())
            {
                std::size_t offset = segmentStart - matches[priorityMatch].searchStartedAt;
                if (prefix.length() > offset && prefix.length())
                {
                    std::size_t len = prefix.length() - offset;
                    unmatched(prefix.substr(offset, len));
                }
            }
            
            matched(rules[priorityMatch], matchStr);
            
            segmentStart = matches[priorityMatch].offsetOfMatch() + matchStr.length();
            
        }while (true);
    }
};


/// parse the ansi color code and change state
void handleANSIString(ColorTokenizer& tok, const std::string& str)
{
    bool brightText = false;
    AnsiColorCode& textCode = tok.lastColorToken;//(AnsiColorCode)0;
    std::smatch match;

    std::string sstring = str;

    static std::regex rx("[[:digit:]]+");

    while (std::regex_search(sstring, match, rx))
    {
       std::stringstream sstr;

       sstr << match.str();
       int result;
       sstr>>result;

       sstring = match.suffix();

       processANSICode(tok, result, brightText, textCode);
    }
}


/// change state based on an integer code in the ansi-code input stream
void processANSICode(ColorTokenizer& tok, int code, bool& brightText, AnsiColorCode& textCode)
{
    switch (code)
    {
        case ANSI_RESET:
            tok.hasBackgroundColor = false;
            break;
        case ANSI_BRIGHT_TEXT:
            brightText = true;
            if (textCode)
            {
                tok.textColor = getAnsiTextColorBright(textCode);
            }
            break;
        case ANSI_BLACK:
        case ANSI_RED:
        case ANSI_GREEN:
        case ANSI_YELLOW:
        case ANSI_BLUE:
        case ANSI_MAGENTA:
        case ANSI_CYAN:
        case ANSI_WHITE:
            textCode = (AnsiColorCode)code;
            
            if (brightText)
            {
                tok.textColor = getAnsiTextColorBright((AnsiColorCode)code);
            }
            else
            {
                tok.textColor = getAnsiTextColor((AnsiColorCode)code);
            }
            break;
        case ANSI_BLACK_BKGRND:
        case ANSI_RED_BKGRND:
        case ANSI_GREEN_BKGRND:
        case ANSI_YELLOW_BKGRND:
        case ANSI_BLUE_BKGRND:
        case ANSI_MAGENTA_BKGRND:
        case ANSI_CYAN_BKGRND:
        case ANSI_WHITE_BKGRND:
            tok.hasBackgroundColor = true;
            tok.backgroundColor = getANSIBackgroundColor((AnsiColorCode)code);
            break;
        default:
            std::cerr<<"unknown ansi code "<<code<<" in output\n";
            return;
    }
}


struct IMGUIQuakeConsole : public Virtuoso::QuakeStyleConsole, public MultiStream, public ColorTokenizer
{
    ConsoleBuf            strb;
    std::ostream          consoleStream;
    char                  InputBuf[256];
    int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
    ImGuiTextFilter       Filter;
    bool                  AutoScroll;
    bool                  ScrollToBottom;
    bool                  formattedText = true;
    
    IMGUIQuakeConsole() : consoleStream(&strb)
    {
        addStream(consoleStream);
        
        ClearLog();
        memset(InputBuf, 0, sizeof(InputBuf));
        HistoryPos = -1;
        
        AutoScroll = true;
        ScrollToBottom = false;
        
        bindMemberCommand("Clear", *this, &IMGUIQuakeConsole::ClearLog, "Clear the console");
        bindCVar("formattedText", formattedText, "");
        
        makeDefaultRules(rules);
    }
    
    ~IMGUIQuakeConsole()
    {
    }
    
    
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
        
        ImGui::TextWrapped("Enter 'help' for help, press TAB to use text completion.");
        
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
        
        defaultFormatting(); // reset formatting at beginning of output
        for (int i = 0; i < strb.items.size(); i++)
        {
            const char* item = strb.items[i].c_str();
            if (!Filter.PassFilter(item))
                continue;
     
            if (formattedText)
            {
                checkRules(strb.items[i]); // < regex color matching
            }
            else
            {
                unmatched(strb.items[i]);
            }
            ImGui::NewLine(); // we know we're printing a single line - we do sameline() between tokens, then new line before we return
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
