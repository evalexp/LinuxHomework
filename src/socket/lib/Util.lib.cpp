#include "./Util.lib.h"

bool Util::startWith(const string &checkStr, const string &startStr)
{
    return checkStr.find(startStr) == 0 ? true : false;
}

bool Util::endWith(const string &checkStr, const string &endStr)
{
    return checkStr.rfind(endStr) == (checkStr.length() - endStr.length()) ? true : false;
}

string Util::replaceAll(const string &src, const string &oldstr, const string &newstr)
{
    string::size_type pos = 0;
    string str = src;
    while ((pos = str.find(oldstr)) != string::npos)
    {
        str.replace(pos, oldstr.length(), newstr);
    }
    return str;
}

string Util::replace(const string &src, const string &oldstr, const string &newstr)
{
    string::size_type pos = 0;
    string str = src;
    if ((pos = str.find(oldstr)) != string::npos)
    {
        str.replace(pos, oldstr.length(), newstr);
    }
    return str;
}

Type Util::resolveOperation(const string &input, string &arg)
{
    Type type;
    if (Util::startWith(input, "/login "))
    {
        arg = Util::replace(input, "/login ", "");
        return Type::LOGIN;
    }
    else if (Util::startWith(input, "/connect "))
    {
        arg = Util::replace(input, "/connect ", "");
        return Type::CONNECT;
    }
    else if (Util::startWith(input, "/join "))
    {
        arg = Util::replace(input, "/join ", "");
        return Type::JOIN;
    }
    else if (Util::startWith(input, "/send "))
    {
        arg = Util::replace(input, "/send ", "");
        return Type::SEND;
    }
    else if (Util::startWith(input, "/sendto "))
    {
        arg = Util::replace(input, "/sendto ", "");
        return Type::SENDTO;
    }
    else if (Util::startWith(input, "/file "))
    {
        arg = Util::replace(input, "/file ", "");
        return Type::SENDFILE;
    }
    else if (Util::startWith(input, "/exit"))
    {
        arg = Util::replace(input, "/exit ", "");
        return Type::EXIT;
    }
    else if (Util::startWith(input, "/help"))
    {
        return Type::HELP;
    }
    else if (Util::startWith(input, "/echo "))
    {
        arg = Util::replace(input, "/echo ", "");
        return Type::ECHO;
    }else if(Util::startWith(input, "/answer")){
        arg = input;
        return Type::ANSWER;
    }
    else
    {
        arg = input;
        return Type::SEND;
    }
}

string Util::trim(const string &src)
{
    if (src.empty())
    {
        return "";
    }
    string t = src;
    return t.erase(0, t.find_first_not_of(" ")).erase(t.find_last_not_of(" ") + 1);
}

string Util::getFileContent(const string& path){
    ifstream fin(path, ios::in);
    stringstream buf;
    if(fin.good()){
        buf << fin.rdbuf();
        fin.close();
        return buf.str();
    }else{
        fin.close();
        throw "File not found.";
    }
}

void Util::transformToSendStruct(string& content){
    content = "[File]" + content + "[FileEOF]";
}