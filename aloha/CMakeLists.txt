check_include_file_cxx(stdint.h HAVE_STDINT_H)
if(HAVE_STDINT_H)
    add_definitions(-DHAVE_STDINT_H)
endif() 

build_lib(
    LIBNAME aloha
    SOURCE_FILES model/aloha-header.cc
                 model/aloha-mac.cc
                 model/aloha-net_device.cc
                 helper/aloha-helper.cc
    HEADER_FILES model/aloha-header.h
                 model/aloha-mac.h
                 model/aloha-net_device.h
                 helper/aloha-helper.h
    LIBRARIES_TO_LINK ${libwireless} ${libapplications} ${libcore} ${libnetwork} ${libinternet} ${libpropagation} ${libmobility}
)