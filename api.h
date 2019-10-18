#ifndef VK_API_H
#define VK_API_H

#include <QObject>
#include <QJsonObject>
#include <QHash>
#include <QtNetwork>

#include <functional>

#include "attachment.h"

namespace VK {

using callback_func_cap = std::function<QString(const QString&)>*;
using callback_func_fa2 = std::function<QString()>*;

using json = QJsonObject;

/* http params */
using params_map = QHash<QString, QString>;

template <typename T>
inline T getValue(const json& obj, const QString& key)
{
    if (obj.contains(key))
    {
        auto universalContainer = obj[key].toVariant();
        if (universalContainer.canConvert<T>())
            return universalContainer.value<T>();
    }
    throw std::runtime_error("Can't extract value from json object");
}

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

    callback_func_cap captcha_callback = nullptr;
    callback_func_fa2 fa2_callback = nullptr;

    inline QString get_captcha_key(const QString &captcha_sid);
    inline QString get_fa2_code();
    QString        data2str(const params_map &data);
    json           str2json(const QString& str);

protected:
    QString             version;
    QString             lang;
    Attachment::User    user;

    bool    check_access();
    QString request(const QString &url, const QString &data);

public:
    Client(const QString _version = "5.65",
           const QString _lang = "en",
           const callback_func_cap cap_callback = nullptr,
           const callback_func_fa2 _fa2_callback = nullptr);

    bool auth(const QString &login, const QString &pass, const QString &access_token = "");

    bool oauth(const callback_func_cap handler);

    json call(const QString &method, const params_map &params);

    json call(const QString &method, const QString &params = "");

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
