#include "attachment.h"
#include "api.h"

namespace VK
{

namespace Attachment
{

const QString Audio::type = "audio";
const QString Photo::type = "photo";
const QString Document::type = "doc";

bool BaseAttachment::parse_type(const jsonObject &data)
{
    try
    {
        parsed_type = getValue<QString>(data, "type");
        return true;
    }
    catch(const std::exception& ex)
    {
        parsed_type.clear();
        qDebug() << ex.what();
    }
    return false;
}

bool BaseAttachment::parse_common(const jsonObject &data)
{
    try
    {
        this->date      = getValue<size_t>(data, "date");
        this->id        = getValue<int>(data, "id");
        this->owner_id  = getValue<int>(data, "owner_id");
        return true;
    }
    catch(const std::exception& ex)
    {
        qDebug() << ex.what();
    }

    return false;
}

bool Audio::parse(const jsonObject &data)
{
    if (data.isEmpty())
        return false;

    try
    {
        if (!parse_type(data) || parsed_type != type)
            return false;

        jsonObject att = getValue<jsonObject>(data, type);
        if (att.isEmpty())
            return false;
        parse_common(att);
        title = getValue<QString>(att, "title");
        artist = getValue<QString>(att, "artist");
        duration = getValue<size_t>(att, "duration");
        direct_url = getValue<QString>(att, "url");
        return true;
    }
    catch(const std::exception& ex)
    {
        qDebug() << ex.what();
    }

    return false;
}

bool Photo::parse(const jsonObject &data)
{
    std::vector<QString> sizes = { "photo_2560", "photo_1280", "photo_807", "photo_604", "photo_130", "photo_75"};
    if (data.isEmpty())
        return false;

    try
    {
        if (!parse_type(data) || parsed_type != type)
            return false;

        jsonObject att = getValue<jsonObject>(data, type);
        if (att.isEmpty())
            return false;
        parse_common(att);
        text = getValue<QString>(att, "text");
        for (const auto& size: sizes)
        {
            if (att.contains(size))
            {
                direct_url = getValue<QString>(att, size);
                break;
            }
        }
        return true;
    }
    catch(const std::exception& ex)
    {
        qDebug() << ex.what();
    }

    return false;
}

QString Document::doc_type_str(const int tp)
{
    switch (tp)
    {
        case 1:
            return "text";
        case 2:
            return "archive";
        case 3:
            return "gif";
        case 4:
            return "image";
        case 5:
            return "audio";
        case 6:
            return "video";
        case 7:
            return "book";
        default:
            return "unknown";
    }
}

bool Document::parse(const jsonObject &data)
{
    if (data.isEmpty())
        return false;

    try
    {
        if (!parse_type(data) || parsed_type != type)
            return false;

        jsonObject att = getValue<jsonObject>(data, type);
        if (att.isEmpty())
            return false;
        parse_common(att);
        title       = getValue<QString> (att, "title");
        ext         = getValue<QString> (att, "ext");
        doc_type    = getValue<int>     (att, "type");
        size        = getValue<size_t>  (att, "size");
        direct_url  = getValue<QString> (att, "url");
        return true;
    }
    catch(const std::exception& ex)
    {
        qDebug() << ex.what();
    }

    return false;
}

bool User::parse(const jsonObject &data)
{
    if (data.isEmpty())
        return false;

    try
    {
        first_name = getValue<QString>(data, "first_name");
        last_name  = getValue<QString>(data, "last_name");
        user_id    = getValue<size_t> (data, "id");
        return true;
    }
    catch(const std::exception& ex)
    {
        qDebug() << ex.what();
    }

    return false;
}

} // Attachment

} // VK
