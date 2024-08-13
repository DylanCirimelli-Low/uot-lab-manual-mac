build_lib_example(
    NAME aloha-simple
    SOURCE_FILES aloha-simple.cc
    LIBRARIES_TO_LINK ${libwireless} ${libaloha} ${libapplications} ${libcore} ${libnetwork} ${libinternet} ${libpropagation} ${libmobility} ${libflow-monitor}
)