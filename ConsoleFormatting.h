//
//  ConsoleFormatting.h
//  VirtuosoConsole
//
//  Created by Virtuoso Engine on 8/31/20.
//

// EndOfLineEscapeStreamScope formatter by Steve132

#ifndef Virtuoso_ConsoleFormatting_h
#define Virtuoso_ConsoleFormatting_h

#include <iostream>
#include <sstream>
#include <iterator>
#include <stack>
#include <regex>


namespace Virtuoso {
namespace io {

// ------------------------------------------------------//
/* --------- End of Line Formatting ------------------- */
// ------------------------------------------------------//
// This modifier wraps the text in a begin / end string pair
// This operates on a single line of code automatically.
// For example with
// EndOfLineEscapeTag HTML {"<HTML>", "</HTML>"};
// cout << HTML << myHtmlString << std::endl;
// will output <HTML> ((myHTMLString)) \n </HTML>

typedef std::pair<std::string, std::string> EndOfLineEscapeTag;

struct EndOfLineEscapeStreamScope
{
protected:

    EndOfLineEscapeTag tag;
    std::ostream& os;

    EndOfLineEscapeStreamScope(const EndOfLineEscapeTag& ttag,std::ostream& tout):
        tag(ttag),
        os(tout)
    {
        os << tag.first; //you can overload this for custom ostream types with a different color interface
        //this might also have a stack interface for if you need multiple resets
    }
    friend EndOfLineEscapeStreamScope operator<<(std::ostream& out,const EndOfLineEscapeTag& tg);

public:

    template<class T>
    EndOfLineEscapeStreamScope& operator<<(T&& t)
    {
        os << std::forward<T>(t);
        return *this;
    }

    EndOfLineEscapeStreamScope& operator<<(std::ostream& (&M)( std::ostream & ))
    {
        M(os);
        return *this;
    }

    ~EndOfLineEscapeStreamScope()
    {
        os << tag.second;
    }
};

inline EndOfLineEscapeStreamScope operator << (std::ostream& os,const EndOfLineEscapeTag& tg)
{
    return EndOfLineEscapeStreamScope(tg,os);
}

// ------------------------------------------------------//
/* --------- Windows terminal setter ------------------- */
// ------------------------------------------------------//

#ifdef _WIN32

/// Win 10 supports ansi color codes in the terminal.  We use this struct to enable them since they're not enabled by default
struct WindowsConsoleInit
{
    BOOl valueSet = FALSE:
    WindowsConsoleInit()
    {
        HANDLE c = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD params = 0;
        GetConsoleMode(c, &params);
        params |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        valueSet = SetConsoleMode(t, params);
    }
};

WindowsConsoleInit g_winConsoleInit; ///< global instance so it'll auto initialize on program start as long as the header is included

#ifdef _WIN32
bool terminalSupportsColorCodes()
{
    return g_winConsoleInit.valueSet;
}
#else
bool terminalSupportsColorCodes()
{
    return true; // mac and linux terminals support out of the box
}

#endif

#endif


struct Rule
{
    std::regex rule;
    ImVec4 color;
    bool hasColor = false;
    ImU32 backgroundColor = 0;
    bool hasBackgroundColor = false;
    bool changeDefaultColors = false;
    bool showToken = true;
    //std::function<void (ColorTokenizer&, const std::string&)> action = DO_NOTHING;
};


typedef std::vector<Rule> RuleSet;


class RegexFormatter
{
    RuleSet rules;

    void matched(Rule& tok, const std::string& str)
    {
       
    }
    
    void unmatched(const std::string& str)
    {
       
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



extern const std::string glsl_qualifiers[];
extern const std::string glsl_keywords[];
extern const std::string glsl_types[];
extern const std::string glsl_functions[];

extern const std::size_t glsl_qualifiers_length;
extern const std::size_t glsl_keywords_length;
extern const std::size_t glsl_types_length;
extern const std::size_t glsl_functions_length;


const std::string glsl_qualifiers[] =
{
"const", "in", "inout", "out", "smooth", "flat", "noperspective", "invariant", "centroid",
    "coherent", "volatile", "restrict", "readonly", "writeonly", "uniform", "buffer", "shared", "sampler", "patch", "binding", "offset", "highp", "mediump", "lowp", "precise",
};


const std::string glsl_keywords[] = {"subroutine", "return", "break", "if", "for", "while", "do", "discard", "continue", "struct", "switch"};


const std::string glsl_types[] =
{
    "void",
    "bool",
    "int",
    "uint",
    "float",
    "double",
    "ivec2", "ivec3", "ivec4",
    "uvec2","uvec3","uvec4",
    "vec2", "vec3","vec4",
    "dvec2","dvec3","dvec4",
    "bvec2", "bvec3", "bvec4",
    "sampler1D","sampler2D", "sampler3D",
    "image1D", "image2D", "image3D",
    "mat2", "mat3", "mat4",
    "mat2x2", "mat2x3", "mat2x4",
    "mat3x2", "mat3x3", "mat3x4",
    "mat4x2", "mat4x3", "mat4x4",
    "dmat2", "dmat3", "dmat4",
    "dmat2x2", "dmat2x3", "dmat2x4",
    "dmat3x2", "dmat3x3", "dmat3x4",
    "dmat4x2", "dmat4x3", "dmat4x4",
    "samplerCube", "imageCube",
    "sampler2DRect", "image2DRect",
    "sampler2DArray", "sampler1DArray", "image1DArray", "image2DArray",
    "samplerBuffer", "imageBuffer",
    "sampler2DMS", "image2DMS", "sampler2DMSArray", "image2DMSArray",
    "samplerCubeArray", "imageCubeArray", "sampler1DShadow", "sampler2DShadow", "sampler2DRectShadow", "sampler1DArrayShadow", "sampler2DArrayShadow", "samplerCubeShadow",
    "samplerCubeArrayShadow",
    "isampler1D","isampler2D", "isampler3D",
    "iimage1D", "iimage2D", "iimage3D",
     "isamplerCube", "iimageCube", "isampler2DRect",
    "iimage2DRect", "isampler1DArray","isampler2DArray", "iimage1DArray", "iimage2DArray","isamplerBuffer", "iimageBuffer", "isampler2DMS", "iimage2DMS", "isampler2DMSArray", "iimage2DMSArray", "isamplerCubeArray", "iimageCubeArray",
    "atomic_uint", "usampler1D","usampler2D", "usampler3D", "uimage1D", "uimage2D", "uimage3D",
    "usamplerCube", "uimageCube", "usampler2DRect", "uimage2DRect", "usampler1DArray","usampler2DArray", "uimage1DArray", "uimage2DArray",
    "usamplerBuffer", "uimageBuffer", "usampler2DMS",
    "uimage2DMS",
    "usampler2DMSArray",
    "uimage2DMSArray", "usamplerCubeArray", "uimageCubeArray"
};


const std::string glsl_functions[] =
{
    "radians", "degrees", "sin", "cos", "tan", "asin", "acos", "atan", "sinh", "cosh", "tanh", "asinh", "acosh", "atanh", "pow", "exp", "log", "exp2", "log2", "sqrt", "inversesqrt", "abs", "sign",
    "floor", "trunc", "round", "roundEven", "ceil", "fract", "mod", "min", "max", "clamp", "mix", "step",
    "smoothstep", "isnan", "isinf", "floatBitsToUint", "floatBitsToInt", "intBitsToFloat", "fma", "frexp",
    "Idexp", "packUnorm2x16", "packSnorm2x16", "unpackUnorm2x16", "unpackSnorm2x16", "unpackUnorm4x8", "unpackSnorm4x8", "packDouble2x32", "unpackDouble2x32", "packHalf2x16", "unpackHalf2x16",
    "length", "distance", "dot", "cross", "normalize", "faceforward", "reflect", "refract", "matrixCompMult", "outerProduct", "transpose", "inverse", "determinant",
    "lessThan", "greaterThan", "lessThanEqual", "greaterThanEqual", "equal", "notEqual", "any", "all", "not", "uaddCarry", "usubBorrow", "umulExtended", "imulExtended", "bitfieldExtract", "bitfieldReverse", "bitfieldInsert", "bitCount", "findLSB", "findMSB", "atomicCounterIncrement",
    "atomicCounterDecrement", "atomicCounter", "atomicCounterOp", "atomicCounterCompSwap", "atomicCounterCompSwap", "atomicOP", "imageSize", "imageSamples", "imageLoad", "imageStore",
    "imageAtomicAdd", "imageAtomicMin", "imageAtomicMax", "imageAtomicAnd", "imageAtomicOr", "imageAtomicXor", "imageAtomicExchange", "imageAtomicCompSwap", "dFdx", "dFdy", "dFdxFine", "dFdyFine", "dFdxCoarse", "dFdyCoarse", "fwidth", "fwidthFine", "fwidthCoarse", "interpolateAtCentroid", "interpolateAtSample", "interpolateAtOffset", "noise1", "noisen",
    "EmitStreamVertex", "EndStreamPrimitive", "EndPrimitive", "EmitVertex", "barrier", "memoryBarrier",
    "groupMemoryBarrier", "memoryBarrierAtomicCounter", "memoryBarrierShared", "memoryBarrierBuffer",
    "memoryBarrierImage", "allInvocationsEqual", "allInvocation", "textureSize", "textureQueryLod", "textureQueryLevels", "textureSamples", "texture", "textureLod", "textureProj", "textureOffset",
    "texelFetch", "texelFetchOffset", "textureProjOffset", "textureLodOffset", "textureProjLod", "textureProjLodOffset", "textureGrad", "textureGradOffset", "textureProjGrad", "textureProjGradOffset","textureGather", "textureGatherOffset", "textureGatherOffsets"
    
};


const std::size_t glsl_qualifiers_length = sizeof(glsl_qualifiers) / sizeof(std::string);
const std::size_t glsl_keywords_length = sizeof(glsl_keywords) / sizeof(std::string);
const std::size_t glsl_types_length = sizeof(glsl_types) / sizeof (std::string);
const std::size_t glsl_functions_length = sizeof(glsl_functions) / sizeof(std::string);


std::string makeKeywordsRegexStr(const std::string keywords[], std::size_t numKeywords)
{
    std::stringstream sstr;
    
    sstr << "\\b" << keywords[0] << "\\b";
    for (int i = 1 ; i < numKeywords; i++)
    {
        sstr << '|' << "\\b"<< keywords[i] <<"\\b";
    }
    
    return (sstr.str());
}


void makeGLSLRules()
{
    {
        Rule r;
        r.rule = std::regex("<glsl>");
        r.hasColor = true;
        r.color =  ImVec4(1.0,0.0,1.0,1.0);
    }
    
    {
        Rule r;
        r.rule = std::regex(makeKeywordsRegexStr(glsl_types, glsl_types_length));
        r.hasColor = true;
        r.color =  ImVec4(0.0,0.0,1.0,1.0);
    }
    
    {
        Rule r;
        r.rule = std::regex(makeKeywordsRegexStr(glsl_keywords, glsl_keywords_length));
        r.hasColor = true;
        r.color =  ImVec4(0.0,0.0,1.0,1.0);
    }
    
    {
        Rule r;
        r.rule =  std::regex(makeKeywordsRegexStr(glsl_functions, glsl_functions_length));
        r.hasColor = true;
        r.color =  ImVec4(0.0,0.0,1.0,1.0);
    }
    
    {
        Rule r;
        r.rule = std::regex(makeKeywordsRegexStr(glsl_qualifiers, glsl_qualifiers_length));
        r.hasColor = true;
        r.color =  ImVec4(0.0,0.0,1.0,1.0);
    }
    
    {
        Rule r;
        r.rule = std::regex("</glsl>");
        r.hasColor = true;
        r.color =  ImVec4(1.0,0.0,1.0,1.0);
    }
}

}
}

#endif /* ConsoleFormatting_h */
