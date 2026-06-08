#include <blip/text/syntax.hpp>
#include <dlfcn.h>
#include <iostream>

extern "C" const TSLanguage *tree_sitter_cpp();

namespace text {

typedef const TSLanguage *(*LanguageFactory)();

SyntaxEngine::SyntaxEngine(const std::string &languageName, const std::string &libraryPath) {
    parser = ts_parser_new();
    libraryHandle = dlopen(libraryPath.c_str(), RTLD_LAZY);

    if (!libraryHandle) {
        std::cerr << "[SyntaxEngine] Failed to load plugin: " << dlerror() << std::endl;
        return;
    }
    std::string functionName = "tree_sitter_" + languageName;
    auto getLanguage = (LanguageFactory)dlsym(libraryHandle, functionName.c_str());
    if (!getLanguage) {
        std::cerr << "[SyntaxEngine] Failed to find function: " << functionName << std::endl;
        dlclose(libraryHandle);
        libraryHandle = nullptr;
        return;
    }
    const TSLanguage *language = getLanguage();
    ts_parser_set_language(parser, language);
    std::cout << "[SyntaxEngine] Successfully loaded " << languageName << " parser!" << std::endl;
}

SyntaxEngine::~SyntaxEngine() {
    if (tree) {
        ts_tree_delete(tree);
    }

    if (parser) {
        ts_parser_delete(parser);
    }

    if (libraryHandle) {
        dlclose(libraryHandle);
    }
}

void SyntaxEngine::parse(const std::string &sourceText) {
    if (tree) {
        ts_tree_delete(tree);
    }
    tree = ts_parser_parse_string(parser, nullptr, sourceText.c_str(), sourceText.length());
}

SDL_Color SyntaxEngine::getColorForByte(uint32_t byteIndex, const config::Theme &theme) {
    if (!tree)
        return theme.foreground;

    TSNode root = ts_tree_root_node(tree);
    TSNode node = ts_node_descendant_for_byte_range(root, byteIndex, byteIndex + 1);

    if (ts_node_is_null(node))
        return theme.foreground;

    const char *nodeType = ts_node_type(node);
    std::string type(nodeType);

    if (type == "string_literal" || type == "system_lib_string")
        return {150, 200, 150, 255};
    if (type == "primitive_type" || type == "type_identifier")
        return {200, 200, 100, 255};
    if (type == "identifier" || type == "function_declarator")
        return {100, 200, 250, 255};
    if (type == "number_literal")
        return {250, 150, 100, 255};
    if (type == "comment")
        return {120, 120, 120, 255};
    if (type == "return" || type == "if" || type == "else" || type == "for" || type == "while")
        return {200, 100, 200, 255};

    return theme.foreground;
}
}
