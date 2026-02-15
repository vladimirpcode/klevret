#include "Command.hpp"

#include "WrapperForParsing.hpp"
#include <stdexcept>
#include <map>
#include <boost/property_tree/ptree.hpp>
#include "command_handlers.hpp"

bool parse_string(const std::string& str){
    WrapperForParsing wrap(str);
    if (wrap.ch != '"'){
        return false;
    }
    wrap.get_next();
    while (std::isalpha(wrap.ch)){
        wrap.get_next();
    }
    if (wrap.ch != '"'){
        return false;
    }
    return true;
}

bool parse_name(const std::string& str){
    //ToDo
    return true;
}

int parse_number(WrapperForParsing& wrap){
    if (!std::isdigit(wrap.ch)){
        throw std::runtime_error("Ошибка парсинга числа: ожидалась цифра");
    }
    int result = 0;
    while (std::isdigit(wrap.ch)){
        result = result * 10 + (wrap.ch - '0');
        wrap.get_next();
    }
    return result;
}

bool parse_ipv4_address(const std::string& str){
    WrapperForParsing wrap(str);
    for (int i = 0; i < 4; ++i){
        try{
            int octet = parse_number(wrap);
            if (octet < 0 || octet > 255){
                return false;
            }
            if (i < 3){
                if (wrap.ch != '.'){
                    return false;
                }
                wrap.get_next();
            }
        } catch(...){
            return false;
        }
    }
    if (wrap.ch != WrapperForParsing::EOT){
        return false;
    }
    return true;
}

bool parse_ipv4_subnet_mask(const std::string& str){
    WrapperForParsing wrap(str);
    int mask = parse_number(wrap);
    if (mask < 0 || mask > 32){
        return false;
    }
    return true;
}

bool parse_ipv6_address(const std::string& str){
    //ToDo
    return true;
}

bool parse_ipv6_subnet_mask(const std::string& str){
    //ToDo
    return true;
}


bool check_command_element(const std::string& str, CommandElement element){
    switch (element.type)
    {
        case CommandElementType::NONE:
            return false;
        case CommandElementType::FIXED_WORD:
            return element.fixed_value == str;
        case CommandElementType::STRING:
            return parse_string(str);
        case CommandElementType::NAME:
            return parse_name(str);
        case CommandElementType::IP_V4_ADDRESS:
            return parse_ipv4_address(str);
        case CommandElementType::IP_V4_SUBNET_MASK:
            return parse_ipv4_subnet_mask(str);
        case CommandElementType::IP_V6_ADDRESS:
            return parse_ipv6_address(str);
        case CommandElementType::IP_V6_SUBNET_MASK:
            return parse_ipv6_subnet_mask(str);

    }
}

std::string to_string(const CommandElement& command_element){
    switch (command_element.type)
    {
        case CommandElementType::FIXED_WORD: return command_element.fixed_value;
        case CommandElementType::IP_V4_ADDRESS: return "<IPv4 address>";
        case CommandElementType::IP_V4_SUBNET_MASK: return "<subnet mask (IPv4)>";
        case CommandElementType::IP_V6_ADDRESS: return "<IPv6 address>";
        case CommandElementType::IP_V6_SUBNET_MASK: return "<subnet mask (IPv6)>";
        case CommandElementType::NAME: return "<name>";
        case CommandElementType::STRING: return "<string>";
        default: return "<UNKNOWN>";
    }
    return "<UNKNOWN>";
}

CommandElement::CommandElement(CommandElementType type, const std::string& fixed_value)
    : type(type), fixed_value(fixed_value)
{

}

CommandDescription::CommandDescription(Language language, const std::string& pattern_description, const std::string& description)
    : language(language), pattern_description(pattern_description), description(description)
{

}




void Command::parse_fixed_word(WrapperForParsing& wrap){
    std::string word = "";
    while (std::isalpha(wrap.ch)){
        word += wrap.ch;
        wrap.get_next();
    }
    CommandElement new_command_element(CommandElementType::FIXED_WORD, word);
    elements.push_back(new_command_element);
}

std::map<std::string, CommandElementType> str_to_element_type_map{
    {"string", CommandElementType::STRING},
    {"name", CommandElementType::NAME},
    {"IPv4Address", CommandElementType::IP_V4_ADDRESS},
    {"IPv4SubnetMask", CommandElementType::IP_V4_SUBNET_MASK},
    {"IPv6Address", CommandElementType::IP_V6_ADDRESS},
    {"IPv6SubnetMask", CommandElementType::IP_V6_SUBNET_MASK},

};

void Command::parse_variable_element(WrapperForParsing& wrap){
    if (wrap.ch != '<'){
        throw std::runtime_error("Ошибка парсинга шаблона команды: ожидалось <");
    }
    wrap.get_next();
    std::string variable_element = "";
    while(std::isalnum(wrap.ch)){
        variable_element += wrap.ch;
        wrap.get_next();
    }
    if (variable_element == ""){
        throw std::runtime_error("Ошибка парсинга шаблона команды: вариативный элемент не может быть пустым <>");
    }
    auto iter = str_to_element_type_map.find(variable_element);
    if (iter == str_to_element_type_map.end()){
        throw std::runtime_error("Ошибка парсинга шаблона команды: неизвестный вариативный элемент <" + variable_element + ">");
    }
    CommandElement new_element(iter->second, "");
    elements.push_back(new_element);
    if (wrap.ch != '>'){
        throw std::runtime_error("Ошибка парсинга шаблона команды: ожидалось <");
    }
    wrap.get_next();
}

void Command::parse_pattern_element(WrapperForParsing& wrap){
    if (std::isalpha(wrap.ch)){
        parse_fixed_word(wrap);
    } else if(wrap.ch == '<'){
        parse_variable_element(wrap);
    } else {
        throw std::runtime_error("Ошибка парсинга шаблона команды: ожидалось слово или <элемент>");
    }
}

void Command::parse_pattern(WrapperForParsing& wrap){
    while (!wrap.is_end()){
        wrap.skip_whitespaces();
        parse_pattern_element(wrap);
    }
}


Command::Command(KlevretComponent klevret_component, const std::string& pattern,
    const std::vector<CommandDescription>& descriptions, command_handler handler)
    :   descriptions(descriptions), handler(handler), klevret_component(klevret_component)
{
    WrapperForParsing wrap(pattern);
    parse_pattern(wrap);
    if (!wrap.is_end()){
        throw std::runtime_error("Ошибка парсинга шаблона команды: ожидался конец текста");
    }
}



std::vector<Command> all_commands = {
    {
        KlevretComponent::CLI,
        "version",
        {
            {Language::Russian, "version", "показать версию ПО Клеврет"},
            {Language::English, "version", "show Klevret software version"}
        },
        cmd_version
    },
    {
        KlevretComponent::DHCP,
        "ip show",
        {

        },
        blank
    },
    {
        KlevretComponent::DHCP,
        "ip address show",
        {

        },
        blank
    },
    {
        KlevretComponent::DHCP,
        "ip address <IPv4Address>",
        {

        },
        cmd_ip_address_ipv4
    },
    {
        KlevretComponent::DHCP,
        "dhcp pool create <IPv4Address> <IPv4Address>",
        {

        },
        cmd_dhcp_pool_create
    },
    {
        KlevretComponent::DHCP,
        "dhcp pool list",
        {

        },
        cmd_dhcp_pool_list
    }

};

std::vector<Command> get_all_commands(){
    return all_commands;
}

void add_command_to_tree(CommandTree& tree_root, const Command& command){
    CommandTree *current_node = &tree_root;
    for (const auto& command_element : command.elements){
        bool found = false;
        for (auto& node : current_node->childs){
            if (node.command_element == command_element){
                found = true;
                current_node = &node;
                break;
            }
        }
        if (!found){
            CommandTree new_node;
            new_node.command_element = command_element;
            current_node->childs.push_back(new_node);
            current_node = &(current_node->childs.at(current_node->childs.size() - 1));
        }
    }
    CommandTree handler;
    handler.command_element.handler = command.handler;
    current_node->childs.push_back(handler);
}

CommandTree create_command_tree(){
    CommandTree tree;
    auto all_commands = get_all_commands();
    for (auto& command : all_commands){
        add_command_to_tree(tree, command);
    }
    return tree;
}
