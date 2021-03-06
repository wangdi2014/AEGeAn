LocusPocus
==========

Introduction
------------

**LocusPocus** is a program for computing *interval loci* (iLoci) from a
provided set gene annotations. Each iLocus corresponds to a single gene, a set
of overlapping genes, or a space between genes. See :doc:`this page <loci>` for
a description of iLoci as an organizational principle for genomics.

Input
-----

Input for LocusPocus is one or more files in GFF3 format (LocusPocus can also
read from standard input if a dash (`-`) is provided as the input filename).
The only strict requirement is that the input must be `valid GFF3`_, although
users should be aware of the common pitfalls described on :doc:`this page
<gff3>`.

The use of `##sequence-region` pragmas is optional, and many GFF3 files do not
include them. LocusPocus uses this information when computing the location of
iLoci at the ends of a sequence. Note that if these pragmas are not declared
explicitly, iLoci will only be reported for sequence regions containing
annotated features.

Users can override `gene` as the default feature of interest, replace it with
one or more other feature types, and construct iLoci for these features in the
same way.

.. _`valid GFF3`: http://sequenceontology.org/resources/gff3.html

Output
------

LocusPocus computes the location of the iLoci from the given gene features and
reports the iLocus locations in GFF3 format. By default, only the iLocus
features themselves are reported, with attributes indicating the number of
genes and transcripts in the locus. Invoking the `--verbose` option enables
reporting of the gene features (and their subfeatures) as well.

Running LocusPocus
------------------

For a complete description of LocusPocus' command-line interface, run the
following command (after AEGeAn has been installed).

.. code-block:: bash

    locuspocus --help
