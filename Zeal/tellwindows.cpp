#include "tellwindows.h"
#include "Zeal.h"
#include "EqAddresses.h"
#include "string_util.h"
#include <regex>

//people will see this commit and be like OMG, then they will see this comment and message the discord channel.
//such hopes and dreams -- its on the list!

bool TellWindows::HandleTell(std::string& cmd_data)
{
    if (Zeal::EqGame::Windows && Zeal::EqGame::Windows->ChatManager)
    {
        Zeal::EqUI::ChatWnd* wnd = Zeal::EqGame::Windows->ChatManager->ChatWindows[Zeal::EqGame::Windows->ChatManager->ActiveChatWnd];
        if (wnd)
        {
            std::string window_title = wnd->Text.Data->Text;
            if (window_title.substr(0,1) == "*")
            {
                cmd_data = "/tell " + window_title.substr(1, window_title.length()-1) + " " + cmd_data;
                return true;
            }
        }
    }
    return false;
}

Zeal::EqUI::ChatWnd* FindTellWnd(std::string& name)
{
    for (int i = 0; i < Zeal::EqGame::Windows->ChatManager->MaxChatWindows; i++)
    {
        Zeal::EqUI::ChatWnd* cwnd = Zeal::EqGame::Windows->ChatManager->ChatWindows[i];
        if (cwnd && cwnd->Text.Data)
        {
            std::string title = cwnd->Text.Data->Text;
            if (title.substr(0, 1) == "*" && Zeal::String::compare_insensitive(title.substr(1, title.length() - 1), name))
                return cwnd;
        }
    }
    return nullptr;
}

std::string GetName(std::string& data)
{
    std::string lower_msg = data;
    std::transform(lower_msg.begin(), lower_msg.end(), lower_msg.begin(), ::tolower);

    // Regex pattern for matching exactly one word before "tells you"
    std::regex tells_pattern(R"((?:^|\]\s*)(\b\w+\b)\s+tells\s+you)");
    // Regex pattern for matching exactly one word after "you told"
    std::regex told_pattern(R"((?:^|\]\s*)you\s+told\s+(\b\w+\b))");

    std::smatch match;

    // Check for "tells you" pattern with only one word before it
    if (std::regex_search(lower_msg, match, tells_pattern)) {
        return match[1].str();  // Return the matched single word before "tells you"
    }
    // Check for "you told" pattern with only one word after it
    else if (std::regex_search(lower_msg, match, told_pattern)) {
        return match[1].str();  // Return the matched single word after "you told"
    }

    return "";  // Return an empty string if no match found
}

void __fastcall AddOutputText(Zeal::EqUI::ChatWnd* wnd, int u, Zeal::EqUI::CXSTR msg, byte channel)
{
    if (channel == 1 || channel==52) //tell channel
    {
        int multiByteSize = WideCharToMultiByte(CP_UTF8, 0, (wchar_t*)msg.Data->Text, -1, NULL, 0, NULL, NULL);
        char* multiByteStr = new char[multiByteSize];
        WideCharToMultiByte(CP_UTF8, 0, (wchar_t*)msg.Data->Text, -1, multiByteStr, multiByteSize, NULL, NULL);
        std::string msg_data = multiByteStr;
        std::string name = GetName(msg_data);
        if (name.length())
        {
            Zeal::EqUI::ChatWnd* tell_window = FindTellWnd(name);
            if (!tell_window)
            {
                std::string WinName = "*" + name;
                Zeal::EqGame::Windows->ChatManager->CreateChatWindow(WinName.c_str(), 0, 3, -1, "", 3);
                tell_window = FindTellWnd(name);
            }

            if (tell_window)
                wnd = tell_window;
        }
    }
    ZealService::get_instance()->hooks->hook_map["AddOutputText"]->original(AddOutputText)(wnd, u, msg, channel);
}

void TellWindows::CleanUI()
{
    if (!Zeal::EqGame::Windows || !Zeal::EqGame::Windows->ChatManager)
        return;
    for (int i = 0; i < Zeal::EqGame::Windows->ChatManager->MaxChatWindows; i++)
    {
        Zeal::EqUI::ChatWnd* cwnd = Zeal::EqGame::Windows->ChatManager->ChatWindows[i];
        if (cwnd && cwnd->Text.Data)
        {
            std::string title = cwnd->Text.Data->Text;
            if (title.substr(0, 1) == "*")
            {
                cwnd->show(0, false);
                break;
            }
        }
    }
}

TellWindows::TellWindows(ZealService* zeal, IO_ini* ini)
{
    zeal->binds_hook->replace_cmd(0x3B, [this](int state)
        {
            if (state && !Zeal::EqGame::EqGameInternal::UI_ChatInputCheck())
            {
                int last_tell_index = *(int*)0x7cf0dc;
                char* reply_to = (char*)(0x7CE45C + (last_tell_index * 0x40));
                std::string reply_to_str = reply_to;
                Zeal::EqUI::ChatWnd* wnd = FindTellWnd(reply_to_str);
                if (wnd && wnd->edit)
                {
                    wnd->edit->SetFocus();
                    return true;
                }
                else
                    return false;
            }
            return false;

        }); //reply hotkey

    zeal->hooks->Add("AddOutputText", 0x4139A2, AddOutputText, hook_type_detour);
    zeal->callbacks->AddGeneric([this]() { CleanUI(); }, callback_type::CleanUI);

}

TellWindows::~TellWindows()
{
}