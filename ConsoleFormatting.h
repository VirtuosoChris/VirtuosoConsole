//
//  ConsoleFormatting.h
//  VirtuosoConsole
//
//  Created by Virtuoso Engine on 8/31/20.
//

#ifndef Virtuoso_ConsoleFormatting_h
#define Virtuoso_ConsoleFormatting_h

#ifdef _WIN32
#include <Windows.h>
#endif

#include <iostream>
#include <sstream>
#include <iterator>
#include <stack>
#include <regex>

namespace Virtuoso
{
namespace io
{

inline constexpr std::string_view ANSI_TEXT_COLOR_RESET = "\u001b[0m";
inline constexpr std::string_view ANSI_TEXT_COLOR_BLACK = "\u001b[30m";
inline constexpr std::string_view ANSI_TEXT_COLOR_RED = "\u001b[31m";
inline constexpr std::string_view ANSI_TEXT_COLOR_GREEN = "\u001b[32m";
inline constexpr std::string_view ANSI_TEXT_COLOR_YELLOW = "\u001b[33m";
inline constexpr std::string_view ANSI_TEXT_COLOR_BLUE = "\u001b[34m";
inline constexpr std::string_view ANSI_TEXT_COLOR_MAGENTA = "\u001b[35m";
inline constexpr std::string_view ANSI_TEXT_COLOR_CYAN = "\u001b[36m";
inline constexpr std::string_view ANSI_TEXT_COLOR_WHITE = "\u001b[37m";
inline constexpr std::string_view ANSI_TEXT_COLOR_BLACK_BRIGHT = "\u001b[30;1m";
inline constexpr std::string_view ANSI_TEXT_COLOR_RED_BRIGHT = "\u001b[31;1m";
inline constexpr std::string_view ANSI_TEXT_COLOR_GREEN_BRIGHT = "\u001b[32;1m";
inline constexpr std::string_view ANSI_TEXT_COLOR_YELLOW_BRIGHT = "\u001b[33;1m";
inline constexpr std::string_view ANSI_TEXT_COLOR_BLUE_BRIGHT = "\u001b[34;1m";
inline constexpr std::string_view ANSI_TEXT_COLOR_MAGENTA_BRIGHT = "\u001b[35;1m";
inline constexpr std::string_view ANSI_TEXT_COLOR_CYAN_BRIGHT = "\u001b[36;1m";
inline constexpr std::string_view ANSI_TEXT_COLOR_WHITE_BRIGHT = "\u001b[37;1m";
inline constexpr std::string_view ANSI_TEXT_COLOR_BLACK_BKGRND = "\u001b[40m";
inline constexpr std::string_view ANSI_TEXT_COLOR_RED_BKGRND = "\u001b[41m";
inline constexpr std::string_view ANSI_TEXT_COLOR_GREEN_BKGRND = "\u001b[42m";
inline constexpr std::string_view ANSI_TEXT_COLOR_YELLOW_BKGRND = "\u001b[43m";
inline constexpr std::string_view ANSI_TEXT_COLOR_BLUE_BKGRND = "\u001b[44m";
inline constexpr std::string_view ANSI_TEXT_COLOR_MAGENTA_BKGRND = "\u001b[45m";
inline constexpr std::string_view ANSI_TEXT_COLOR_CYAN_BKGRND = "\u001b[46m";
inline constexpr std::string_view ANSI_TEXT_COLOR_WHITE_BKGRND = "\u001b[47m";

// ------------------------------------------------------//
/* --------- End of Line Formatting ------------------- */
// ------------------------------------------------------//
// This modifier wraps the text in a begin / end string pair
// This operates on a single line of code automatically.
// For example with
// EndOfLineEscapeTag HTML {"<HTML>", "</HTML>"};
// cout << HTML << myHtmlString << std::endl;
// will output <HTML> ((myHTMLString)) \n </HTML>
//
// EndOfLineEscapeStreamScope formatter by Steve132

typedef std::pair<std::string, std::string> EndOfLineEscapeTag;

struct EndOfLineEscapeStreamScope
{
  protected:
    EndOfLineEscapeTag tag;
    std::ostream &os;

    EndOfLineEscapeStreamScope(const EndOfLineEscapeTag &ttag, std::ostream &tout) : tag(ttag),
                                                                                     os(tout)
    {
        os << tag.first; //you can overload this for custom ostream types with a different color interface
        //this might also have a stack interface for if you need multiple resets
    }
    friend EndOfLineEscapeStreamScope operator<<(std::ostream &out, const EndOfLineEscapeTag &tg);

  public:
    template <class T>
    EndOfLineEscapeStreamScope &operator<<(T &&t)
    {
        os << std::forward<T>(t);
        return *this;
    }

    EndOfLineEscapeStreamScope &operator<<(std::ostream &(&M)(std::ostream &))
    {
        M(os);
        return *this;
    }

    ~EndOfLineEscapeStreamScope()
    {
        os << tag.second;
    }
};

inline EndOfLineEscapeStreamScope operator<<(std::ostream &os, const EndOfLineEscapeTag &tg)
{
    return EndOfLineEscapeStreamScope(tg, os);
}



// ------------------------------------------------------//
/* --------- Streambuf Swapper-------------------------- */
// ------------------------------------------------------//

/// streambuf_swapper
/// Steve132
/// RAII method of swapping a global iostream buffer
/// https://stackoverflow.com/questions/14860267/what-can-go-wrong-if-cout-rdbuf-is-used-to-switch-buffer-and-never-set-it-back
struct streambuf_swapper {
    std::ios &m_s;
    std::streambuf *m_buf;

    streambuf_swapper(std::ios &s, std::streambuf *buf) : m_s(s), m_buf(s.rdbuf(buf)) {}
    ~streambuf_swapper() { m_s.rdbuf(m_buf); }
};

// ------------------------------------------------------//
/* --------- Windows terminal setter ------------------- */
// ------------------------------------------------------//

#ifdef _WIN32

/// Win 10 supports ansi color codes in the terminal.  We use this struct to enable them since they're not enabled by default
struct WindowsConsoleInit
{
    BOOL  valueSet = FALSE;
    WindowsConsoleInit()
    {
        HANDLE c = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD params = 0;
        GetConsoleMode(c, &params);
        params |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        valueSet = SetConsoleMode(c, params);
    }
};

extern WindowsConsoleInit g_winConsoleInit; ///< global instance so it'll auto initialize on program start as long as the header is included

#ifdef _WIN32

inline bool terminalSupportsColorCodes()
{
    return g_winConsoleInit.valueSet;
}
#else
inline bool terminalSupportsColorCodes()
{
    return true; // mac and linux terminals support out of the box
}

#endif

#endif


struct RegexFormatter
{
    struct Rule
    {
        std::regex rule;
        std::function<std::string (const std::string&)> filter = DO_NOTHING;

        inline static std::string DO_NOTHING(const std::string& s){return s;}
    };

    typedef std::vector<Rule> RuleSet;
    
    RuleSet rules;

    std::string matched(Rule &tok, const std::string &str)
    {
        return tok.filter(str);
    }

    std::string unmatched(const std::string &str)
    {
        return str;
    }

    std::string format(const std::string &str)
    {
        std::stringstream sstr;
        
        struct SearchParams
        {
            std::smatch match;
            std::size_t searchStartedAt = 0;
            std::string result;
            std::string prefix;
            std::string suffix;
            bool hasResult = true;

            std::size_t offsetOfMatch() const
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
                SearchParams &match = matches[i];

                if (!match.hasResult)
                    continue;

                // update the next search result for the regex we chose, or for any regex whose latest result
                // is now behind the edge of the string we've consumed
                if ((i == lastPriorityMatch) || (match.offsetOfMatch() < segmentStart) || (!segmentStart))
                {
                    match.hasResult = std::regex_search((str.begin() + segmentStart), str.end(), match.match, rules[i].rule);

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
                sstr << unmatched(str.substr(segmentStart, str.size() - segmentStart));

                return sstr.str();
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
                    sstr << unmatched(prefix.substr(offset, len));
                }
            }

            sstr << matched(rules[priorityMatch], matchStr);

            segmentStart = matches[priorityMatch].offsetOfMatch() + matchStr.length();

        } while (true);
    }
};


inline std::string makeKeywordsRegexStr(const std::string keywords[], std::size_t numKeywords)
{
    std::stringstream sstr;

    sstr << "\\b" << keywords[0] << "\\b";
    for (int i = 1; i < numKeywords; i++)
    {
        sstr << '|' << "\\b" << keywords[i] << "\\b";
    }

    return (sstr.str());
}

// use std bind to set this as a 'filter' for regex formatter.  see guiTest.cpp example in 'demos' folder for example glsl syntax highlighting
inline std::string highlightKeyword(const std::string& format, const std::string& str)
{
    return format + str + "\u001b[0m";
}

// ----------------------
// -------- MISC --------
// ----------------------

struct membuf: std::streambuf
{
    membuf(const char* begin, const char* end)
    {
         this->setg((char*)begin, (char*)begin, (char*)end);
    }
};


} // namespace io
} // namespace Virtuoso

#endif /* ConsoleFormatting_h */

#if defined(ConsoleFormatting_Implementation)
namespace Virtuoso
{
    namespace io
    {
        WindowsConsoleInit g_winConsoleInit; ///< global instance so it'll auto initialize on program start as long as the header is included
    }
}
#endif

