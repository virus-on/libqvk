#include "messages.h"

namespace VK
{

bool DialogInfo::parse(const jsonObject &data)
{
    if (data.isEmpty())
        return false;

    try
    {
        title = getValue<QString>(data, "title");
        if (title == " ... " || title.isEmpty())
            title.clear();
        body = getValue<QString>(data, "body");
        if (data.contains("chat_id"))
        {
            chat_id = getValue<int>(data, "chat_id");
            is_chat = true;
        }
        else
        {
            chat_id = getValue<int>(data, "user_id");
            is_chat = false;
        }
        if (body.isEmpty())
            body = "...";
        return true;
    }
    catch (const std::exception& ex)
    {
        qDebug() << ex.what();
    }

    return false;
}

vector_dialogs Messages::get_dialogs(const size_t count, const size_t offset)
{
    VK::vector_dialogs res;

    if (count == 0 || count > 200) {
        return res;
    }

    VK::params_map params = {
        {"count", QString::number(count)},
        {"offset", QString::number(offset)},
        {"preview_length", "1"},
    };

    jsonObject jres = call("messages.getDialogs", params);
    if (jres.isEmpty())
        return res;

    if (!jres.contains("error"))
    {
        QJsonArray items = getValue<QJsonArray>(getValue<jsonObject>(jres, "response"), "items");

        for (auto it = items.begin(); it != items.end(); it++)
        {
            jsonObject item = items.at(it.i).toObject();
            if (!item.contains("message"))
                continue;
            item = getValue<jsonObject>(item, "message");
            VK::DialogInfo dialog;
            if (dialog.parse(item))
            {
                if (dialog.title.isEmpty())
                    dialog.title = dialog.is_chat ? get_chat_title(dialog.chat_id) : get_username(dialog.chat_id);
            }
            res.push_back(std::move(dialog));
        }
    }

    return res;
}

vector_dialogs Messages::get_all_dialogs(const size_t max_count)
{
    const size_t count = (max_count > 200 || max_count < 1) ? 200 : max_count;
    vector_dialogs all_res, tmp_res;
    size_t offset = 0;
    for(;;)
    {
        tmp_res = this->get_dialogs(count, offset);
        if (tmp_res.size() < 1)
            break;

        std::move(tmp_res.begin(), tmp_res.end(), std::back_inserter(all_res));
        offset += count;
        if (max_count > 0 && max_count <= all_res.size()) {
            break;
        }
    }

    return all_res;
}

QString Messages::get_chat_title(const int chat_id)
{
    if (chat_id < chat_offset)
        return "";

    params_map params = {
        {"chat_ids", QString::number(chat_id - chat_offset)},
    };

    jsonObject jres = call("messages.getChat", params);
    if (jres.isEmpty() || jres.contains("error"))
        return "";
    jsonObject info = getValue<jsonObject>(jres, "response");
    info = jres.begin().value().toObject();
    QString tmp = getValue<QString>(info, "title");
    return tmp;
}

QString Messages::get_username(const int user_id)
{
    params_map params = {
        {"user_ids", QString::number(user_id)},
    };

    jsonObject jres = call("users.get", params);
    if (jres.isEmpty() || jres.contains("error"))
        return "";
    Attachment::User user;
    jsonObject info = getValue<jsonObject>(jres, "response");
    if (info.isEmpty())
        return "";
    info = jres.begin().value().toObject();
    user.parse(info);
    return user.first_name + " " + user.last_name;
}

template<typename T>
std::vector<T> Messages::get_attachments(const int chat_id, const size_t count)
{
    std::vector<T> res;
    size_t real_count = (count > 200 || count < 1) ? 200 : count;
    QString next;
    VK::params_map params = {
        {"peer_id", QString::number(chat_id)},
        {"media_type", T::type},
        {"start_from", next},
        {"count", QString::number(real_count)}
    };

    try
    {
        for (;;)
        {
            jsonObject jres = call("messages.getHistoryAttachments", params);
            if (jres.isEmpty())
                return res;

            size_t parsed = 0;
            if (!jres.contains("error"))
            {
                next = getValue<QString>(getValue<jsonObject>(jres, "response"), "next_from");

                QJsonArray items = getValue<QJsonArray>(getValue<jsonObject>(jres, "response"), "items");
                for (auto it = items.begin(); it != items.end(); it++)
                {
                    jsonObject item = items.at(it.i).toObject();
                    if (!item.contains("attachment"))
                        continue;
                    item = getValue<jsonObject>(item, "attachment");
                    T att;
                    if (att.parse(item))
                    {
                        res.push_back(att);
                        parsed++;
                    }
                }
            }

            if((count == 0 && next.isEmpty())
                || (count > 0 && res.size() >= count)
                || (parsed < 1))
            {
                break;
            }

            params["start_from"] = next;
        }
    }
    catch (const std::exception& ex)
    {
        qDebug() << ex.what();
    }

    return res;
}

} // VK

template std::vector<VK::Attachment::Audio>
VK::Messages::get_attachments<VK::Attachment::Audio>(const int , const size_t);

template std::vector<VK::Attachment::Photo>
VK::Messages::get_attachments<VK::Attachment::Photo>(const int , const size_t);

template std::vector<VK::Attachment::Document>
VK::Messages::get_attachments<VK::Attachment::Document>(const int , const size_t);
