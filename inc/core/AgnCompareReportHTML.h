/**

Copyright (c) 2010-2014, Daniel S. Standage and CONTRIBUTORS

The AEGeAn Toolkit is distributed under the ISC License. See
the 'LICENSE' file in the AEGeAn source code distribution or
online at https://github.com/standage/AEGeAn/blob/master/LICENSE.

**/
#ifndef AEGEAN_COMPARE_REPORT_HTML
#define AEGEAN_COMPARE_REPORT_HTML

#include "core/logger_api.h"
#include "extended/node_visitor_api.h"
#include "AgnLocus.h"

/**
 * @class AgnCompareReportHTML
 *
 * The ``AgnCompareReportHTML`` class is an extension of the
 * ``AgnCompareReport`` class. This node visitor relies on its parent class to
 * process a stream of ``AgnLocus`` objects (containing two alternative sources
 * of annotation to be compared) and then produces textual reports of the
 * comparison statistics.
 */
typedef struct AgnCompareReportHTML AgnCompareReportHTML;

/**
 * @functype By default, the ParsEval summary report includes an overview with
 * the start time, filenames, and command-line arguments. Users can override
 * this behavior by specifying a callback function that follows this signature.
 */
typedef void (*AgnCompareReportHTMLOverviewFunc)(FILE *outstream, void *data);


/**
 * @function After the node stream has been processed, call this function to
 * write a summary of all locus comparisons to the output directory.
 */
void agn_compare_report_html_create_summary(AgnCompareReportHTML *rpt);

/**
 * @function Class constructor. Creates a node visitor used to process a stream
 * of ``AgnLocus`` objects containing two sources of annotation to be compared.
 * Reports will be written in ``outdir`` and status messages will be written
 * to the logger.
 */
GtNodeVisitor *agn_compare_report_html_new(const char *outdir, bool gff3,
                                           AgnLocusPngMetadata *pngdata,
                                           GtLogger *logger);

/**
 * @function By default, the summary report's title will be 'ParsEval Summary'.
 *  Use this function to replace the title text.
 */
void agn_compare_report_html_reset_summary_title(AgnCompareReportHTML *rpt,
                                                 GtStr *title_string);

/**
 * @function Specify a callback function to be used when printing an overview
 * on the summary report.
 */
void agn_compare_report_html_set_overview_func(AgnCompareReportHTML *rpt,
                                         AgnCompareReportHTMLOverviewFunc func,
                                         void *funcdata);

#endif
