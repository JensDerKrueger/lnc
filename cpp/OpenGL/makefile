# use make to compile all projects on linux or mac
# on linux you need the following:
# sudo apt install -y libglu1-mesa-dev freeglut3-dev mesa-common-dev xorg-dev mesa-utils libglfw3-dev libglew-dev libomp-dev

TOPTARGETS := all clean release

SUBDIRS := $(wildcard */.)
SUBDIRS := $(filter-out VS/. Utils/. LibML/. Network/.,$(SUBDIRS))
FIRSTDIR := LibML/. Network/.
UTILSDIR := Utils/.

$(TOPTARGETS): $(SUBDIRS)

$(SUBDIRS): $(FIRSTDIR)
	$(MAKE) -C $@ $(MAKECMDGOALS)

$(FIRSTDIR): $(UTILSDIR)
	$(MAKE) -C $@ $(MAKECMDGOALS)

$(UTILSDIR):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)
.PHONY: $(TOPTARGETS) $(FIRSTDIR)
.PHONY: $(TOPTARGETS) $(UTILSDIR)
