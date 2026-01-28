#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>

extern "C" {
    #include "tree_sitter/api.h"
}

extern "C" const TSLanguage *tree_sitter_python();

// Simple data structures to hold extracted information
struct Parameter {
    std::string name;
    std::string type = "unknown";  // Always unknown for now
};

struct Function {
    std::string name;
    std::vector<Parameter> parameters;
    std::string return_type = "unknown";
    bool is_async = false;
};

struct Method {
    std::string name;
    std::vector<Parameter> parameters;
    bool is_async = false;
};

struct Class {
    std::string name;
    std::vector<Method> methods;
};

struct Module {
    std::string name;
    std::vector<Function> functions;
    std::vector<Class> classes;
};

// Helper function to extract text from a node
std::string get_node_text(TSNode node, const std::string& source_code) {
    uint32_t start = ts_node_start_byte(node);
    uint32_t end = ts_node_end_byte(node);
    return source_code.substr(start, end - start);
}

// Check if a name is public (doesn't start with underscore)
bool is_public(const std::string& name) {
    return !name.empty() && name[0] != '_';
}

// JSON output helpers
void print_parameter_json(const Parameter& param, bool is_last) {
    std::cout << "        {\n";
    std::cout << "          \"name\": \"" << param.name << "\",\n";
    std::cout << "          \"type\": { \"name\": \"" << param.type << "\" }\n";
    std::cout << "        }" << (is_last ? "\n" : ",\n");
}

void print_function_json(const Function& func, bool is_last) {
    std::cout << "      {\n";
    std::cout << "        \"name\": \"" << func.name << "\",\n";
    std::cout << "        \"signature\": {\n";
    std::cout << "          \"ret\": { \"type\": { \"name\": \"" << func.return_type << "\" } },\n";
    std::cout << "          \"args\": [\n";
    
    for (size_t i = 0; i < func.parameters.size(); i++) {
        print_parameter_json(func.parameters[i], i == func.parameters.size() - 1);
    }
    
    std::cout << "          ]\n";
    std::cout << "        },\n";
    std::cout << "        \"async\": " << (func.is_async ? "true" : "false") << "\n";
    std::cout << "      }" << (is_last ? "\n" : ",\n");
}

void print_method_json(const Method& method, bool is_last) {
    std::cout << "          {\n";
    std::cout << "            \"name\": \"" << method.name << "\",\n";
    std::cout << "            \"async\": " << (method.is_async ? "true" : "false") << "\n";
    std::cout << "          }" << (is_last ? "\n" : ",\n");
}

void print_class_json(const Class& cls, bool is_last) {
    std::cout << "      {\n";
    std::cout << "        \"name\": \"" << cls.name << "\",\n";
    std::cout << "        \"methods\": [\n";
    
    for (size_t i = 0; i < cls.methods.size(); i++) {
        print_method_json(cls.methods[i], i == cls.methods.size() - 1);
    }
    
    std::cout << "        ]\n";
    std::cout << "      }" << (is_last ? "\n" : ",\n");
}

void print_module_json(const Module& module) {
    std::cout << "{\n";
    std::cout << "  \"name\": \"" << module.name << "\",\n";
    std::cout << "  \"scope\": {\n";
    std::cout << "    \"funcs\": [\n";
    
    for (size_t i = 0; i < module.functions.size(); i++) {
        print_function_json(module.functions[i], i == module.functions.size() - 1);
    }
    
    std::cout << "    ],\n";
    std::cout << "    \"classes\": [\n";
    
    for (size_t i = 0; i < module.classes.size(); i++) {
        print_class_json(module.classes[i], i == module.classes.size() - 1);
    }
    
    std::cout << "    ]\n";
    std::cout << "  }\n";
    std::cout << "}\n";
}

// ============================================================================
// AST Traversal - Extracts functions, classes, and methods
// ============================================================================

// Visit nodes and collect data into a Module structure
void visit_node(TSNode node, const std::string& source_code, Module& module, Class* current_class = nullptr) {
    const char* type = ts_node_type(node);
    
    // Handle class definitions
    if (strcmp(type, "class_definition") == 0) {
        TSNode name_node = ts_node_child_by_field_name(node, "name", 4);
        if (!ts_node_is_null(name_node)) {
            Class cls;
            cls.name = get_node_text(name_node, source_code);
            
            // Only process public classes
            if (is_public(cls.name)) {
                // Visit children to collect methods
                uint32_t child_count = ts_node_child_count(node);
                for (uint32_t i = 0; i < child_count; i++) {
                    TSNode child = ts_node_child(node, i);
                    visit_node(child, source_code, module, &cls);
                }
                
                module.classes.push_back(cls);
            }
            return;  // Don't recurse further, we handled children above
        }
    }
    
    // Handle function definitions
    else if (strcmp(type, "function_definition") == 0) {
        TSNode name_node = ts_node_child_by_field_name(node, "name", 4);
        if (!ts_node_is_null(name_node)) {
            std::string func_name = get_node_text(name_node, source_code);
            
            // Only process public functions/methods
            if (!is_public(func_name)) {
                return;  // Skip private functions
            }
            
            // Extract parameters
            std::vector<Parameter> params;
            TSNode params_node = ts_node_child_by_field_name(node, "parameters", 10);
            if (!ts_node_is_null(params_node)) {
                uint32_t param_count = ts_node_child_count(params_node);
                for (uint32_t i = 0; i < param_count; i++) {
                    TSNode child = ts_node_child(params_node, i);
                    const char* child_type = ts_node_type(child);
                    
                    Parameter param;
                    
                    if (strcmp(child_type, "identifier") == 0) {
                        param.name = get_node_text(child, source_code);
                        params.push_back(param);
                    }
                    else if (strcmp(child_type, "typed_parameter") == 0) {
                        TSNode param_name = ts_node_child_by_field_name(child, "name", 4);
                        if (!ts_node_is_null(param_name)) {
                            param.name = get_node_text(param_name, source_code);
                            params.push_back(param);
                        }
                    }
                    else if (strcmp(child_type, "default_parameter") == 0) {
                        TSNode param_name = ts_node_child_by_field_name(child, "name", 4);
                        if (!ts_node_is_null(param_name)) {
                            param.name = get_node_text(param_name, source_code);
                            params.push_back(param);
                        }
                    }
                }
            }
            
            // If inside a class, it's a method
            if (current_class != nullptr) {
                Method method;
                method.name = func_name;
                method.parameters = params;
                current_class->methods.push_back(method);
            }
            // Otherwise it's a top-level function
            else {
                Function func;
                func.name = func_name;
                func.parameters = params;
                module.functions.push_back(func);
            }
            
            return;  // Don't recurse into function body
        }
    }
    
    // Recursively visit all children
    uint32_t child_count = ts_node_child_count(node);
    for (uint32_t i = 0; i < child_count; i++) {
        TSNode child = ts_node_child(node, i);
        visit_node(child, source_code, module, current_class);
    }
}

int main() {
    // Create parser
    TSParser *parser = ts_parser_new();
    ts_parser_set_language(parser, tree_sitter_python());
    
    // Read Python file
    std::ifstream file("../test.py");
    if (!file.is_open()) {
        std::cerr << "Error: Could not open test.py" << std::endl;
        return 1;
    }
    
    std::string source_code((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
    file.close();
    
    // Parse
    TSTree *tree = ts_parser_parse_string(
        parser,
        nullptr,
        source_code.c_str(),
        source_code.length()
    );
    
    // Create module and collect data
    Module module;
    module.name = "test.py";
    
    TSNode root_node = ts_tree_root_node(tree);
    visit_node(root_node, source_code, module);
    
    // Output JSON
    print_module_json(module);
    
    // Cleanup
    ts_tree_delete(tree);
    ts_parser_delete(parser);
    
    return 0;
}