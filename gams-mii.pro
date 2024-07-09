TEMPLATE = subdirs

SUBDIRS += src
SUBDIRS += tests

src.file = src/mii.pro
tests.depends = src
