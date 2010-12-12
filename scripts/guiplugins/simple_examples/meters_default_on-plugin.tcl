# This makes the meters in the Pd window be on by default the meters
# have been removed from Pure Data/Vanilla, so this only works on
# Pd-extended.

set ::meters 1
pdsend "pd meters 1"
