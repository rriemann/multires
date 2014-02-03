TEMPLATE = subdirs
SUBDIRS  = node \
           regular \
           rawRunner \
           guiRunner

# http://blog.rburchell.com/2013/10/every-time-you-configordered-kitten-dies.html
rawRunner.depends = node
guiRunner.depends = node
