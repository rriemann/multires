TEMPLATE = subdirs
SUBDIRS  = \
           multires \
           monores \
           rawRunner \
           guiRunner \
           # compaRunner

# http://blog.rburchell.com/2013/10/every-time-you-configordered-kitten-dies.html

guiRunner.depends = monores # multires
compaRunner.depends = monores multires

contains(DEFINES, REGULAR) {
    rawRunner.depends = monores
} else {
    rawRunner.depends = multires
}
