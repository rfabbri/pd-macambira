
'many' is a library of techniques for creating and managing many instances of an object

- bundle: a bundle of instances all connected to the same inlets and outlets
- instances: many instances that are both individually and globally addressable 
- instances~: same as [instances] but with outlet~s
- polypoly~: designed for MIDI polyphony, built upon [poly] and based on [polypoly]
- voices~: similar to polypoly~, but for generate voice allocation without being tied to MIDI

Objects in the 'many' lib are based on code from Steven Pickles' (aka pix) nqpoly4 and Frank Barknecht's polypoly.  Thanks to pix's nqpoly4 because that was the real groundbreaking work that proved that a useful and reliable instance-managing object could be programmed in Pd.

- Hans-Christoph Steiner <hans@eds.org>

