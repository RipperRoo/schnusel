TEMPLATE = subdirs
SUBDIRS = \
    conproxy \
    conview \
    winapitools

conview.depends = conproxy
conproxy.depends = winapitools
