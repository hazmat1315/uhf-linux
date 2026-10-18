#include "mpv/mpv_backend.h"

#include <mpv/client.h>

#include <QtGlobal>

#include <clocale>

namespace uhf::mpv {

namespace {

QByteArray urlToMpv(const QUrl& url)
{
    if (url.isLocalFile())
        return url.toLocalFile().toUtf8();
    return url.toString(QUrl::FullyEncoded).toUtf8();
}

} // namespace

MpvBackend::MpvBackend()
{
    // libmpv requires the C numeric locale; Qt may have changed it.
    std::setlocale(LC_NUMERIC, "C");

    m_mpv = mpv_create();
    if (!m_mpv) {
        qWarning("mpv_create returned null");
        return;
    }

    // Quiet, self-contained core. Rendering into a Qt surface is layered on
    // separately; until then the core decodes headless (no A/V device needed),
    // which also keeps the smoke test deterministic.
    mpv_set_option_string(m_mpv, "config", "no");
    mpv_set_option_string(m_mpv, "terminal", "no");
    mpv_set_option_string(m_mpv, "idle", "yes");
    mpv_set_option_string(m_mpv, "vo", "null");
    mpv_set_option_string(m_mpv, "ao", "null");
    // Stay loaded at end-of-file so metadata (e.g. duration) remains readable.
    mpv_set_option_string(m_mpv, "keep-open", "yes");

    const int rc = mpv_initialize(m_mpv);
    if (rc < 0)
        qWarning("mpv_initialize failed: %d (%s)", rc, mpv_error_string(rc));
    m_initialized = rc >= 0;
}

MpvBackend::~MpvBackend()
{
    if (m_mpv) {
        mpv_terminate_destroy(m_mpv);
        m_mpv = nullptr;
    }
}

void MpvBackend::setHeadless(bool headless)
{
    m_headless = headless;
}

void MpvBackend::load(const core::StreamRequest& request)
{
    if (!m_mpv)
        return;

    if (!request.userAgent.isEmpty())
        mpv_set_property_string(m_mpv, "user-agent", request.userAgent.toUtf8().constData());
    if (!request.referrer.isEmpty())
        mpv_set_property_string(m_mpv, "referrer", request.referrer.toUtf8().constData());

    const QByteArray url = urlToMpv(request.url);
    const char* cmd[] = {"loadfile", url.constData(), nullptr};
    mpv_command(m_mpv, cmd);
}

void MpvBackend::play()
{
    if (!m_mpv)
        return;
    int flag = 0;
    mpv_set_property(m_mpv, "pause", MPV_FORMAT_FLAG, &flag);
}

void MpvBackend::pause()
{
    if (!m_mpv)
        return;
    int flag = 1;
    mpv_set_property(m_mpv, "pause", MPV_FORMAT_FLAG, &flag);
}

void MpvBackend::stop()
{
    if (!m_mpv)
        return;
    const char* cmd[] = {"stop", nullptr};
    mpv_command(m_mpv, cmd);
}

double MpvBackend::doubleProperty(const QString& name, double fallback) const
{
    if (!m_mpv)
        return fallback;
    double value = fallback;
    if (mpv_get_property(m_mpv, name.toUtf8().constData(), MPV_FORMAT_DOUBLE, &value) >= 0)
        return value;
    return fallback;
}

} // namespace uhf::mpv
