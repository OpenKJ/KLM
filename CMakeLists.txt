cmake_minimum_required(VERSION 3.5)
project(klm LANGUAGES CXX C)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
option(BUILD_SHARED_LIBS "" NO)
add_subdirectory(quazip)
add_compile_definitions(
        TAGLIB_STATIC
)
include_directories(
        taglib/taglib
        taglib/taglib/toolkit
        taglib/taglib/mpeg/id3v2
        taglib/taglib/mpeg/id3v2/frames
        taglib/taglib/mpeg/id3v1
        taglib/taglib/mod
        taglib/taglib/ogg
        taglib/taglib/ogg/flac
        taglib/taglib/ogg/opus
        taglib/taglib/ogg/speex
        taglib/taglib/ogg/vorbis
        taglib/taglib/flac
        taglib/taglib/ape
        taglib/taglib/riff
        taglib/taglib/asf
        taglib/taglib/mpeg
        taglib/taglib/riff
        taglib/taglib/riff/aiff
        taglib/taglib/riff/wav
        taglib/taglib/it
        taglib/taglib/mp4
        taglib/taglib/mpc
        taglib/taglib/s3m
        taglib/taglib/trueaudio
        taglib/taglib/wavpack
        taglib/taglib/xm
)
set(TAGLIB_SOURCES
        taglib/taglib/ape/apefile.cpp
        taglib/taglib/ape/apefooter.cpp
        taglib/taglib/ape/apeitem.cpp
        taglib/taglib/ape/apeproperties.cpp
        taglib/taglib/ape/apetag.cpp
        taglib/taglib/asf/asfattribute.cpp
        taglib/taglib/asf/asffile.cpp
        taglib/taglib/asf/asfpicture.cpp
        taglib/taglib/asf/asfproperties.cpp
        taglib/taglib/asf/asftag.cpp
        taglib/taglib/flac/flacfile.cpp
        taglib/taglib/flac/flacmetadatablock.cpp
        taglib/taglib/flac/flacpicture.cpp
        taglib/taglib/flac/flacproperties.cpp
        taglib/taglib/flac/flacunknownmetadatablock.cpp
        taglib/taglib/it/itfile.cpp
        taglib/taglib/it/itproperties.cpp
        taglib/taglib/mod/modfile.cpp
        taglib/taglib/mod/modfilebase.cpp
        taglib/taglib/mod/modproperties.cpp
        taglib/taglib/mod/modtag.cpp
        taglib/taglib/mp4/mp4atom.cpp
        taglib/taglib/mp4/mp4coverart.cpp
        taglib/taglib/mp4/mp4file.cpp
        taglib/taglib/mp4/mp4item.cpp
        taglib/taglib/mp4/mp4properties.cpp
        taglib/taglib/mp4/mp4tag.cpp
        taglib/taglib/mpc/mpcfile.cpp
        taglib/taglib/mpc/mpcproperties.cpp
        taglib/taglib/mpeg/id3v1/id3v1genres.cpp
        taglib/taglib/mpeg/id3v1/id3v1tag.cpp
        taglib/taglib/mpeg/id3v2/frames/attachedpictureframe.cpp
        taglib/taglib/mpeg/id3v2/frames/chapterframe.cpp
        taglib/taglib/mpeg/id3v2/frames/commentsframe.cpp
        taglib/taglib/mpeg/id3v2/frames/eventtimingcodesframe.cpp
        taglib/taglib/mpeg/id3v2/frames/generalencapsulatedobjectframe.cpp
        taglib/taglib/mpeg/id3v2/frames/ownershipframe.cpp
        taglib/taglib/mpeg/id3v2/frames/podcastframe.cpp
        taglib/taglib/mpeg/id3v2/frames/popularimeterframe.cpp
        taglib/taglib/mpeg/id3v2/frames/privateframe.cpp
        taglib/taglib/mpeg/id3v2/frames/relativevolumeframe.cpp
        taglib/taglib/mpeg/id3v2/frames/synchronizedlyricsframe.cpp
        taglib/taglib/mpeg/id3v2/frames/tableofcontentsframe.cpp
        taglib/taglib/mpeg/id3v2/frames/textidentificationframe.cpp
        taglib/taglib/mpeg/id3v2/frames/uniquefileidentifierframe.cpp
        taglib/taglib/mpeg/id3v2/frames/unknownframe.cpp
        taglib/taglib/mpeg/id3v2/frames/unsynchronizedlyricsframe.cpp
        taglib/taglib/mpeg/id3v2/frames/urllinkframe.cpp
        taglib/taglib/mpeg/id3v2/id3v2extendedheader.cpp
        taglib/taglib/mpeg/id3v2/id3v2footer.cpp
        taglib/taglib/mpeg/id3v2/id3v2frame.cpp
        taglib/taglib/mpeg/id3v2/id3v2framefactory.cpp
        taglib/taglib/mpeg/id3v2/id3v2header.cpp
        taglib/taglib/mpeg/id3v2/id3v2synchdata.cpp
        taglib/taglib/mpeg/id3v2/id3v2tag.cpp
        taglib/taglib/mpeg/mpegfile.cpp
        taglib/taglib/mpeg/mpegheader.cpp
        taglib/taglib/mpeg/mpegproperties.cpp
        taglib/taglib/mpeg/xingheader.cpp
        taglib/taglib/ogg/flac/oggflacfile.cpp
        taglib/taglib/ogg/opus/opusfile.cpp
        taglib/taglib/ogg/opus/opusproperties.cpp
        taglib/taglib/ogg/speex/speexfile.cpp
        taglib/taglib/ogg/speex/speexproperties.cpp
        taglib/taglib/ogg/vorbis/vorbisfile.cpp
        taglib/taglib/ogg/vorbis/vorbisproperties.cpp
        taglib/taglib/ogg/oggfile.cpp
        taglib/taglib/ogg/oggpage.cpp
        taglib/taglib/ogg/oggpageheader.cpp
        taglib/taglib/ogg/xiphcomment.cpp
        taglib/taglib/riff/aiff/aifffile.cpp
        taglib/taglib/riff/aiff/aiffproperties.cpp
        taglib/taglib/riff/wav/infotag.cpp
        taglib/taglib/riff/wav/wavfile.cpp
        taglib/taglib/riff/wav/wavproperties.cpp
        taglib/taglib/riff/rifffile.cpp
        taglib/taglib/s3m/s3mfile.cpp
        taglib/taglib/s3m/s3mproperties.cpp
        taglib/taglib/toolkit/tbytevector.cpp
        taglib/taglib/toolkit/tbytevectorlist.cpp
        taglib/taglib/toolkit/tbytevectorstream.cpp
        taglib/taglib/toolkit/tdebug.cpp
        taglib/taglib/toolkit/tdebuglistener.cpp
        taglib/taglib/toolkit/tfile.cpp
        taglib/taglib/toolkit/tfilestream.cpp
        taglib/taglib/toolkit/tiostream.cpp
        taglib/taglib/toolkit/tpropertymap.cpp
        taglib/taglib/toolkit/trefcounter.cpp
        taglib/taglib/toolkit/tstring.cpp
        taglib/taglib/toolkit/tstringlist.cpp
        taglib/taglib/toolkit/tzlib.cpp
        taglib/taglib/toolkit/unicode.cpp
        taglib/taglib/trueaudio/trueaudiofile.cpp
        taglib/taglib/trueaudio/trueaudioproperties.cpp
        taglib/taglib/wavpack/wavpackfile.cpp
        taglib/taglib/wavpack/wavpackproperties.cpp
        taglib/taglib/xm/xmfile.cpp
        taglib/taglib/xm/xmproperties.cpp
        taglib/taglib/audioproperties.cpp
        taglib/taglib/fileref.cpp
        taglib/taglib/tag.cpp
        taglib/taglib/tagunion.cpp
        taglib/taglib/tagutils.cpp
        taglib/taglib/ape/apefile.h
        taglib/taglib/ape/apefooter.h
        taglib/taglib/ape/apeitem.h
        taglib/taglib/ape/apeproperties.h
        taglib/taglib/ape/apetag.h
        taglib/taglib/asf/asfattribute.h
        taglib/taglib/asf/asffile.h
        taglib/taglib/asf/asfpicture.h
        taglib/taglib/asf/asfproperties.h
        taglib/taglib/asf/asftag.h
        taglib/taglib/asf/asfutils.h
        taglib/taglib/flac/flacfile.h
        taglib/taglib/flac/flacmetadatablock.h
        taglib/taglib/flac/flacpicture.h
        taglib/taglib/flac/flacproperties.h
        taglib/taglib/flac/flacunknownmetadatablock.h
        taglib/taglib/it/itfile.h
        taglib/taglib/it/itproperties.h
        taglib/taglib/mod/modfile.h
        taglib/taglib/mod/modfilebase.h
        taglib/taglib/mod/modfileprivate.h
        taglib/taglib/mod/modproperties.h
        taglib/taglib/mod/modtag.h
        taglib/taglib/mp4/mp4atom.h
        taglib/taglib/mp4/mp4coverart.h
        taglib/taglib/mp4/mp4file.h
        taglib/taglib/mp4/mp4item.h
        taglib/taglib/mp4/mp4properties.h
        taglib/taglib/mp4/mp4tag.h
        taglib/taglib/mpc/mpcfile.h
        taglib/taglib/mpc/mpcproperties.h
        taglib/taglib/mpeg/id3v1/id3v1genres.h
        taglib/taglib/mpeg/id3v1/id3v1tag.h
        taglib/taglib/mpeg/id3v2/frames/attachedpictureframe.h
        taglib/taglib/mpeg/id3v2/frames/chapterframe.h
        taglib/taglib/mpeg/id3v2/frames/commentsframe.h
        taglib/taglib/mpeg/id3v2/frames/eventtimingcodesframe.h
        taglib/taglib/mpeg/id3v2/frames/generalencapsulatedobjectframe.h
        taglib/taglib/mpeg/id3v2/frames/ownershipframe.h
        taglib/taglib/mpeg/id3v2/frames/podcastframe.h
        taglib/taglib/mpeg/id3v2/frames/popularimeterframe.h
        taglib/taglib/mpeg/id3v2/frames/privateframe.h
        taglib/taglib/mpeg/id3v2/frames/relativevolumeframe.h
        taglib/taglib/mpeg/id3v2/frames/synchronizedlyricsframe.h
        taglib/taglib/mpeg/id3v2/frames/tableofcontentsframe.h
        taglib/taglib/mpeg/id3v2/frames/textidentificationframe.h
        taglib/taglib/mpeg/id3v2/frames/uniquefileidentifierframe.h
        taglib/taglib/mpeg/id3v2/frames/unknownframe.h
        taglib/taglib/mpeg/id3v2/frames/unsynchronizedlyricsframe.h
        taglib/taglib/mpeg/id3v2/frames/urllinkframe.h
        taglib/taglib/mpeg/id3v2/id3v2extendedheader.h
        taglib/taglib/mpeg/id3v2/id3v2footer.h
        taglib/taglib/mpeg/id3v2/id3v2frame.h
        taglib/taglib/mpeg/id3v2/id3v2framefactory.h
        taglib/taglib/mpeg/id3v2/id3v2header.h
        taglib/taglib/mpeg/id3v2/id3v2synchdata.h
        taglib/taglib/mpeg/id3v2/id3v2tag.h
        taglib/taglib/mpeg/mpegfile.h
        taglib/taglib/mpeg/mpegheader.h
        taglib/taglib/mpeg/mpegproperties.h
        taglib/taglib/mpeg/mpegutils.h
        taglib/taglib/mpeg/xingheader.h
        taglib/taglib/ogg/flac/oggflacfile.h
        taglib/taglib/ogg/opus/opusfile.h
        taglib/taglib/ogg/opus/opusproperties.h
        taglib/taglib/ogg/speex/speexfile.h
        taglib/taglib/ogg/speex/speexproperties.h
        taglib/taglib/ogg/vorbis/vorbisfile.h
        taglib/taglib/ogg/vorbis/vorbisproperties.h
        taglib/taglib/ogg/oggfile.h
        taglib/taglib/ogg/oggpage.h
        taglib/taglib/ogg/oggpageheader.h
        taglib/taglib/ogg/xiphcomment.h
        taglib/taglib/riff/aiff/aifffile.h
        taglib/taglib/riff/aiff/aiffproperties.h
        taglib/taglib/riff/wav/infotag.h
        taglib/taglib/riff/wav/wavfile.h
        taglib/taglib/riff/wav/wavproperties.h
        taglib/taglib/riff/rifffile.h
        taglib/taglib/riff/riffutils.h
        taglib/taglib/s3m/s3mfile.h
        taglib/taglib/s3m/s3mproperties.h
        taglib/taglib/toolkit/taglib.h
        taglib/taglib/toolkit/tbytevector.h
        taglib/taglib/toolkit/tbytevectorlist.h
        taglib/taglib/toolkit/tbytevectorstream.h
        taglib/taglib/toolkit/tdebug.h
        taglib/taglib/toolkit/tdebuglistener.h
        taglib/taglib/toolkit/tfile.h
        taglib/taglib/toolkit/tfilestream.h
        taglib/taglib/toolkit/tiostream.h
        taglib/taglib/toolkit/tlist.h
        taglib/taglib/toolkit/tmap.h
        taglib/taglib/toolkit/tpropertymap.h
        taglib/taglib/toolkit/trefcounter.h
        taglib/taglib/toolkit/tstring.h
        taglib/taglib/toolkit/tstringlist.h
        taglib/taglib/toolkit/tutils.h
        taglib/taglib/toolkit/tzlib.h
        taglib/taglib/toolkit/unicode.h
        taglib/taglib/trueaudio/trueaudiofile.h
        taglib/taglib/trueaudio/trueaudioproperties.h
        taglib/taglib/wavpack/wavpackfile.h
        taglib/taglib/wavpack/wavpackproperties.h
        taglib/taglib/xm/xmfile.h
        taglib/taglib/xm/xmproperties.h
        taglib/taglib/audioproperties.h
        taglib/taglib/fileref.h
        taglib/taglib/tag.h
        taglib/taglib/taglib_export.h
        taglib/taglib/tagunion.h
        taglib/taglib/tagutils.h
        )

option(WITH_ZLIB "Build with ZLIB" OFF)

add_subdirectory(spdlog)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(WINDOWS_RC_ICON
            icons/appicon.rc
            )
endif()

add_executable(klm
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        karaokefile.cpp
        karaokefile.h
        dupefindercrc.cpp
        dupefindercrc.h
        dupefinderat.cpp
        dupefinderat.h
        qcrc32.cpp
        qcrc32.h
        images.qrc
        ${TAGLIB_SOURCES}
        ${WINDOWS_RC_ICON}
        )

target_link_libraries(klm PRIVATE Qt${QT_VERSION_MAJOR}::Widgets spdlog::spdlog_header_only QuaZip::QuaZip)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set_property(
            TARGET klm
            PROPERTY WIN32_EXECUTABLE true
    )
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    install(
            TARGETS klm
            DESTINATION bin
    )
    install(
            FILES linux/klm.svg
            DESTINATION share/pixmaps
    )
    install(
            FILES linux/klm.desktop
            DESTINATION share/applications
    )
endif ()