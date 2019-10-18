#ifndef VK_ATTACHMENT_H
#define VK_ATTACHMENT_H

#include <QJsonObject>
#include <QJsonDocument>

namespace VK {

using jsonObject    = QJsonObject;
using jsonArray     = QJsonArray;
using jsonValue     = QJsonValue;
using jsonDocument  = QJsonDocument;

/* List of VK Attacment, e.g.: Audio, Photo, Document
 */

namespace Attachment {

/* class for store data
 */
class DataModel {
protected:
    bool parsed;
public:
    /* parse jsonObject and fill self fileds
     * if all is ok returned true
     */
    virtual bool parse(const jsonObject &data) = 0;

    /* return class fields in std::string
     */
    virtual QString dump() = 0;

    inline bool is_parsed() {
        return parsed;
    }

    virtual ~DataModel() {}
};


/* VK User Info
 */
class User : public DataModel {
public:

    QString first_name;
    QString last_name;
    size_t user_id;

    bool parse(const jsonObject &data);
    QString dump() {
        return QString::number(user_id) + " - " + first_name + " " + last_name;
    }

    virtual ~User() {}
};


/* class for store common data of VK Attacment
 */
class BaseAttachment : public DataModel {
protected:
    QString parsed_type;
    bool parse_type(const jsonObject &data);
    bool parse_common(const jsonObject &data);
public:
    int id;
    int owner_id;

    size_t date; /* timestamp date attachment */

    QString direct_url; /* url to download attachment */

    virtual ~BaseAttachment() {}
};


/* VK Audio Attachment
 */
class Audio : public BaseAttachment {
public:
    static const QString type; /* need to make request to API */
    QString artist;
    QString title;
    size_t duration; /* in seconds */
    bool parse(const jsonObject &data);
    QString dump() {
        return artist + " - " + title + " : " + QString::number(duration);
    }

    virtual ~Audio() {}
};


/* VK Photo Attachment
 */
class Photo : public BaseAttachment {
public:
    static const QString type; /* need to make request to API */
    QString text;
    bool parse(const jsonObject &data);
    QString dump() {
        return text + " - " + direct_url + " : " + QString::number(date);
    }

    virtual ~Photo() {}
};


/* VK Document Attachment
 */
class Document : public BaseAttachment {
public:
    static const QString type; /* need to make request to API */
    QString title;
    QString ext; /* for e.g. mp3, gif, jpg */
    int doc_type = -1;
    static QString doc_type_str(const int tp);
    size_t size = 0; /* in byte */

    bool parse(const jsonObject &data);
    QString dump() {
        return title + " - " + ext + " : " + QString::number(size);
    }

    virtual ~Document() {}
};

} // namespace Attachment

} // namespace VK

#endif // VK_ATTACHMENT_H
