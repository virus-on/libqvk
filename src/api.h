#ifndef VK_API_H
#define VK_API_H

#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <QHash>
#include <QtNetwork>

#include <functional>

#include "attachment.h"


using jsonObject    = QJsonObject;
using jsonArray     = QJsonArray;
using jsonValue     = QJsonValue;
using jsonDocument  = QJsonDocument;

template <typename T>
inline T getValue(const jsonObject& obj, const QString& key)
{
    if (obj.contains(key))
    {
        auto universalContainer = obj[key].toVariant();
        if (universalContainer.canConvert<T>())
            return universalContainer.value<T>();
    }
    throw std::runtime_error("Can't extract value from jsonObject object");
}

template <>
inline jsonArray getValue(const jsonObject& obj, const QString& key)
{
    if (obj.contains(key))
    {
        jsonValue val = obj[key];
        if (val.isArray())
        {
            return val.toArray();
        }
    }
    throw std::runtime_error("Can't extract JsonArray from jsonObject");
}

template <>
inline jsonObject getValue(const jsonObject& obj, const QString& key)
{
    if (obj.contains(key))
    {
        jsonValue val = obj[key];
        if (val.isObject())
        {
            return val.toObject();
        }
    }
    throw std::runtime_error("Can't extract JsonObject from jsonObject");
}


namespace VK {

using callback_func_cap = std::function<QString(const QString&)>;
using callback_func_fa2 = std::function<QString()>;

/* http params */
using params_map = QHash<QString, QString>;

class Client
{

private:
    static const QString app_id;
    static const QString app_secret;
    static const QString scope;
    static const QString auth_url;
    static const QString api_url;

    QString a_t;
    QString captcha_sid;
    QString captcha_key;
    QString fa2_code;

    QString l_error;

    callback_func_cap captcha_callback;
    callback_func_fa2 fa2_callback;

    inline QString get_captcha_key(const QString &_captcha_sid);
    inline QString get_fa2_code();
    QString        data2str(const params_map &data);
    jsonObject           str2json(const QString& str);

protected:
    QString             version;
    QString             lang;
    Attachment::User    user;

    bool    check_access();
    QString request(const QString &url, const QString &data);

public:
    Client(const QString _version = "5.65",
           const QString _lang = "en",
           const callback_func_cap _cap_callback = nullptr,
           const callback_func_fa2 _fa2_callback = nullptr);

    bool auth(const QString &login, const QString &pass, const QString &access_token = "");

    bool oauth(const callback_func_cap handler);

    jsonObject call(const QString &method, const params_map &params);

    jsonObject call(const QString &method, const QString &params = "");

    void clear();

    QString first_name() const;
    QString last_name() const;
    size_t  user_id() const;

    QString access_token() const;
    QString last_error() const;

    void set_fa2_callback(const callback_func_fa2 _fa2_callback = nullptr);
    void set_cap_callback(const callback_func_cap cap_callback = nullptr);

    virtual ~Client() {}
};

} // namespace VK

#endif // VK_API_H
