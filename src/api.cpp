#include "api.h"

namespace VK {

const QString Client::api_url   = "https://api.vk.com/method/";
const QString Client::app_id    = "3140623";// android=2274003
const QString Client::app_secret= "VeWdmVclDCtn6ihuP1nt";// android=hHbZxrka2uZ6jB1inYsH
const QString Client::scope     = "offline,groups,messages,friends,audio";
const QString Client::auth_url  = "https://oauth.vk.com/token";

QString Client::get_captcha_key(const QString &captcha_sid)
{
    return (captcha_callback != nullptr) ? (*captcha_callback)(captcha_sid) : "";
}

QString Client::get_fa2_code()
{
    return (fa2_callback != nullptr) ? (*fa2_callback)() : "";
}

QString Client::data2str(const params_map &data)
{
    QString params;

    for (auto& key: data.keys())
    {
        params += key + "=" + QUrl::toPercentEncoding(data[key]) + "&";
    }

    return params;
}

jsonObject Client::str2json(const QString &str)
{
    QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8());
    if (!doc.isNull() && doc.isObject())
        return doc.object();
    else
        throw std::runtime_error("Invalid document recieved!");
}

bool Client::check_access()
{
    jsonObject jres = call("users.get", "");
    if(jres.find("error") != jres.end()) {
        this->clear();
        return false;
    }
    try {
        jsonArray  info = getValue<jsonArray>(jres, "response");
        jsonObject infoObj = info.begin()->toObject();
        user.parse(infoObj);
    }
    catch(const std::exception& ex) {
        qDebug() << ex.what();
        this->clear();
        return false;
    }

    return true;
}

QString Client::request(const QString &url, const QString &data)
{
    QString wholeUrl = url + (data.isEmpty() ? "" : "?" ) + data;
    QString replyBody;
    qDebug() << wholeUrl;

    QNetworkAccessManager* mgr = new QNetworkAccessManager();
    QNetworkReply *rep = mgr->get(QNetworkRequest(wholeUrl));
    while (!rep->isFinished())
        QCoreApplication::processEvents();

    if (rep->error() != QNetworkReply::NoError)
    {
        qDebug() << rep->errorString();
        return "";
    }

    QByteArray bts = rep->readAll();
    replyBody = bts;
    qDebug() << "Got reply:" << replyBody;

    mgr->deleteLater();
    return replyBody;
}

Client::Client(const QString _version, const QString _lang, const callback_func_cap cap_callback, const callback_func_fa2 _fa2_callback)
    : captcha_callback(cap_callback)
    , fa2_callback(_fa2_callback)
    , version(_version), lang(_lang) { }

bool Client::auth(const QString &login, const QString &pass, const QString &access_token)
{
    if (!access_token.isEmpty())
    {
        this->a_t = access_token;
        if (check_access())
            return true;
    }
    this->a_t.clear();

    if (login.isEmpty() || pass.isEmpty())
        return false;

    params_map params = {
            {"client_id", app_id},
            {"grant_type", "password"},
            {"client_secret", app_secret},
            {"scope", scope},
            {"username", login},
            {"password", pass},
    };

    if  (!captcha_sid.isEmpty())
    {
        params.insert("captcha_sid", captcha_sid);
        params.insert("captcha_key", captcha_key);
    }
    if (fa2_callback != nullptr)
        params.insert("2fa_supported", "1");
    if (!fa2_code.isEmpty())
        params.insert("code", fa2_code);

    QString data = data2str(params);
    captcha_sid.clear();
    captcha_key.clear();
    fa2_code.clear();

    QString res = request(auth_url, data);
    if (res.isEmpty())
        return false;

    try
    {
        jsonObject jres = str2json(res);
        if (!jres.contains("error") || jres.contains("access_token"))
        {
            a_t = getValue<QString>(jres, "access_token");
            user.user_id = getValue<size_t>(jres, "user_id");

            return check_access();
        }

        l_error = getValue<QString>(jres, "error");

        if (l_error == "need_validation")
        {
            fa2_code = get_fa2_code();
            if (!fa2_code.isEmpty())
                return this->auth(login, pass);
        }
        else if (l_error == "need_captcha")
        {
            captcha_sid = getValue<QString>(jres, "captcha_sid");
            captcha_key = get_captcha_key(captcha_sid);
            if (!captcha_key.isEmpty())
                return this->auth(login, pass);
        }
    }
    catch (const std::exception& ex)
    {
        qDebug() << ex.what();
    }

    return false;
}

bool Client::oauth(const callback_func_cap handler)
{
    if (handler == nullptr)
        return false;

    this->clear();
    QString oauth_url = "https://oauth.vk.com/authorize";
    params_map params = {
            {"client_id", app_id},
            {"display", "page"},
            {"redirect_uri", "https://oauth.vk.com/blank.html"},
            {"scope", scope},
            {"response_type", "token"},
            {"v", version},
    };

    oauth_url += data2str(params);
    QString blank = (*handler)(oauth_url);
    if (blank.isEmpty())
        return false;

    auto idx = blank.indexOf("=");
    if (idx == -1)
        return false;
    idx++;
    a_t = blank.mid(idx);

    idx = a_t.indexOf("&expires_in");
    if (idx == -1) {
        this->clear();
        return false;
    }
    a_t = a_t.mid(0, idx);

    return !a_t.isEmpty();
}

jsonObject Client::call(const QString &method, const params_map &params)
{
    if (method.isEmpty())
        return jsonObject();

    QString data;
    if (params.size() > 0)
        data = data2str(params);

    return call(method, data);
}

jsonObject Client::call(const QString &method, const QString &params)
{
    if (method.isEmpty())
        return jsonObject();
    QString url = api_url + method;
    QString data = params + (params.isEmpty() ? "" : "&");

    params_map tmp_params;
    if (!captcha_sid.isEmpty())
    {
        tmp_params.insert("captcha_sid", captcha_sid);
        tmp_params.insert("captcha_key", captcha_key);
    }
    tmp_params.insert("v", version);
    tmp_params.insert("lang", lang);
    if (!a_t.isEmpty())
        tmp_params.insert("access_token", a_t);

    data += data2str(tmp_params);
    captcha_sid.clear();
    captcha_key.clear();

    QString res = request(url, data);
    if (res.isEmpty())
        return jsonObject();

    try
    {
        jsonObject jres = str2json(res);

        if (!jres.contains("error"))
            return jres;

        jsonObject item = getValue<jsonObject>(jres, "error");
        l_error = getValue<QString>(item, "error_msg");

        if (l_error == "need_captcha")
        {
            captcha_sid = getValue<QString>(item, "captcha_sid");
            captcha_key = get_captcha_key(captcha_sid);
            if (!captcha_key.isEmpty())
                return this->call(method, params);
        }
    }
    catch (const std::exception& ex)
    {
        qDebug() << ex.what();
    }

    return jsonObject();
}

void Client::clear()
{
    a_t.clear();
    user.first_name.clear();
    user.last_name.clear();
    user.user_id = 0;

    captcha_sid.clear();
    captcha_key.clear();
    fa2_code.clear();
}

QString Client::first_name() const
{
    return user.first_name;
}

QString Client::last_name() const
{
    return user.last_name;
}

size_t Client::user_id() const
{
    return user.user_id;
}

QString Client::access_token() const
{
    return a_t;
}

QString Client::last_error() const
{
    return l_error;
}

void Client::set_fa2_callback(const callback_func_fa2 _fa2_callback)
{
    fa2_callback = _fa2_callback;
}

void Client::set_cap_callback(const callback_func_cap cap_callback)
{
    captcha_callback = cap_callback;
}

} // namespace VK
