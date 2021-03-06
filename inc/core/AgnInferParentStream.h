/**

Copyright (c) 2010-2015, Daniel S. Standage and CONTRIBUTORS

The AEGeAn Toolkit is distributed under the ISC License. See
the 'LICENSE' file in the AEGeAn source code distribution or
online at https://github.com/standage/AEGeAn/blob/master/LICENSE.

**/

#ifndef AEGEAN_INFER_PARENT_STREAM
#define AEGEAN_INFER_PARENT_STREAM

#include "extended/node_stream_api.h"
#include "core/hashmap_api.h"
#include "AgnUnitTest.h"

/**
 * @class AgnInferParentStream
 *
 * Implements the GenomeTools ``GtNodeStream`` interface. This node stream
 * creates new features as parents for the specified types. For example, if
 * ``type_parents`` includes an entry with ``tRNA`` as the key and ``gene`` as
 * the value, this node stream will create a ``gene`` feature for any ``tRNA``
 * feature that lacks a gene parent.
 */
typedef struct AgnInferParentStream AgnInferParentStream;

/**
 * @function Class constructor. The hashmap contains a list of key-value pairs,
 * both strings. Any time the stream encounters a top-level (parentless) feature
 * whose type is a key in the hashmap, a parent will be created for this feature
 * of the type associated with the key.
 */
GtNodeStream* agn_infer_parent_stream_new(GtNodeStream *in_stream,
                                          GtHashmap *type_parents);

/**
 * @function Set the source (GFF3 2nd column) for all inferred features.
 */
void agn_infer_parent_stream_set_source(AgnInferParentStream *stream,
                                        GtStr *source);

/**
 * @function Run unit tests for this class. Returns true if all tests passed.
 */
bool agn_infer_parent_stream_unit_test(AgnUnitTest *test);

#endif
