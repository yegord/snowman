/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <QString>

namespace nc {

class Branding {
    QString applicationName_;
    QString applicationVersion_;
    QString organizationDomain_;
    QString organizationName_;
    QString licenseName_;
    QString licenseUrl_;
    QString reportBugsTo_;

public:
    const QString &applicationName() const { return applicationName_; }
    void setApplicationName(QString name) { applicationName_ = std::move(name); }

    const QString &applicationVersion() const { return applicationVersion_; }
    void setApplicationVersion(QString version) { applicationVersion_ = std::move(version); }

    const QString &organizationDomain() const { return organizationDomain_; }
    void setOrganizationDomain(QString domain) { organizationDomain_ = std::move(domain); }

    const QString &organizationName() const { return organizationName_; }
    void setOrganizationName(QString name) { organizationName_ = std::move(name); }

    const QString &licenseName() const { return licenseName_; }
    void setLicenseName(QString name) { licenseName_ = std::move(name); }

    const QString &licenseUrl() const { return licenseUrl_; }
    void setLicenseUrl(QString url) { licenseUrl_ = std::move(url); }

    const QString &reportBugsTo() const { return reportBugsTo_; }
    void setReportBugsTo(QString reportBugsTo) { reportBugsTo_ = reportBugsTo; }
};

/**
 * \return Branding of the Nc library.
 */
Branding branding();

} // namespace nc

/* vim:set et sts=4 sw=4: */
