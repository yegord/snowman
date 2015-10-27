/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "BfdParser.h"

#include <QCoreApplication> /* For Q_DECLARE_TR_FUNCTIONS. */
#include <QFile>

#include <nc/common/Foreach.h>
#include <nc/common/LogToken.h>
#include <nc/common/Range.h>
#include <nc/common/make_unique.h>

#include <nc/core/image/Image.h>
#include <nc/core/image/Reader.h>
#include <nc/core/image/Relocation.h>
#include <nc/core/image/Section.h>
#include <nc/core/input/ParseError.h>
#include <nc/core/input/Utils.h>

#define PACKAGE 1 /* Work-around for bfd.h */
#define PACKAGE_VERSION 1 /* Work-around for bfd.h */

#include "bfd.h"

namespace nc {
namespace input {
namespace bfdparser {

namespace {

using nc::core::input::read;
using nc::core::input::getAsciizString;
using nc::core::input::ParseError;

class BfdParserImpl {
    Q_DECLARE_TR_FUNCTIONS(BfdParserImpl)

    QIODevice *source_;
    core::image::Image *image_;
    const LogToken &log_;

    ByteOrder byteOrder_;
    std::vector<const core::image::Section *> sections_;
    std::vector<const core::image::Symbol *> symbols_;

public:
    BfdParserImpl(QIODevice *source, core::image::Image *image, const LogToken &log):
        source_(source), image_(image), log_(log), byteOrder_(ByteOrder::Current)
    {}

    void parse() {
    	bfd *ibfd = nullptr;
		QFile *file = static_cast<QFile *>(source_);
		QString filename = file->fileName();
		source_->seek(0); /* Convention */

		if((ibfd = bfd_openr(filename.toAscii().data(), nullptr)) == nullptr){
			throw ParseError(tr("Could not open file: %1").arg(filename));
		}
  
  		/* Here we try to figure out the target. */
  		if (!bfd_check_format(ibfd, bfd_object)){
			bfd_close(ibfd);
			throw ParseError(tr("Could not open file: %1").arg(filename));
		}
  
  		int arch = bfd_get_arch(ibfd);
  		byteOrder_ = bfd_big_endian(ibfd) ? ByteOrder::BigEndian : ByteOrder::LittleEndian;
  	
  		switch (arch) {
            case bfd_arch_i386:
                if(bfd_get_mach(ibfd) != bfd_mach_x86_64){
	                image_->platform().setArchitecture(QLatin1String("i386"));
                } else {
                	image_->platform().setArchitecture(QLatin1String("x86-64"));
                }
                break;
            case bfd_arch_arm:
                if (byteOrder_ == ByteOrder::LittleEndian) {
                    image_->platform().setArchitecture(QLatin1String("arm-le"));
                } else {
                    image_->platform().setArchitecture(QLatin1String("arm-be"));
                }
                break;
            case bfd_arch_mips:
                if (byteOrder_ == ByteOrder::LittleEndian) {
					if(bfd_arch_bits_per_address(ibfd) == 32) {
                        image_->platform().setArchitecture(QLatin1String("mips-le"));
                    } else {
                    	image_->platform().setArchitecture(QLatin1String("mips64-le"));
                    }
                } 
                else if(bfd_arch_bits_per_address(ibfd) == 32) {
                    image_->platform().setArchitecture(QLatin1String("mips-be"));
                } else {
                	image_->platform().setArchitecture(QLatin1String("mips64-be"));
                }
                break;
            default:
            	const char *id = bfd_printable_name(ibfd);
            	bfd_close(ibfd);
                throw ParseError(tr("Unknown machine id: %1.").arg(id));
        }
    }

private:

};

} // anonymous namespace

BfdParser::BfdParser():
    core::input::Parser(QLatin1String("BFD"))
{}

bool BfdParser::doCanParse(QIODevice *source) const {
	bfd *ibfd = nullptr;
	QFile *file = static_cast<QFile *>(source);
	QString filename = file->fileName();
	
	bfd_init();
	ibfd = bfd_openr(filename.toAscii().data(), nullptr);
	if(ibfd == nullptr){
		return false;
	}
	if (!bfd_check_format(ibfd, bfd_object)){
		bfd_close(ibfd);
		return false;
	}
	bfd_close(ibfd);
	return true;
}

void BfdParser::doParse(QIODevice *source, core::image::Image *image, const LogToken &log) const {
	BfdParserImpl(source, image, log).parse();
}

} // namespace bfdparser
} // namespace input
} // namespace nc

/* vim:set et sts=4 sw=4: */

