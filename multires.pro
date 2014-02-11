TEMPLATE = subdirs
SUBDIRS  = node \
           regular \
           rawRunner \
           guiRunner \
           compaRunner

# http://blog.rburchell.com/2013/10/every-time-you-configordered-kitten-dies.html

guiRunner.depends = regular node
compaRunner.depends = regular node

contains(DEFINES, REGULAR) {
    rawRunner.depends = regular
} else {
    rawRunner.depends = node
}
