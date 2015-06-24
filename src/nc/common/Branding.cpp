/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "Branding.h"

#include "Version.h"

namespace nc {

Branding branding() {
    Branding result;

    result.setApplicationName(QLatin1String("Nc"));
    result.setApplicationVersion(QLatin1String(version));
    result.setOrganizationDomain(QLatin1String("derevenets.com"));
    result.setOrganizationName(result.organizationDomain());
    result.setLicenseName(licenseName);
    result.setLicenseUrl(licenseUrl);
    result.setReportBugsTo(reportBugsTo);

    return result;
}

} // namespace nc

/* vim:set et sts=4 sw=4: */
