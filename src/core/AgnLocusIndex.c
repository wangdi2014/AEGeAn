#include <omp.h>
#include "AgnLocus.h"
#include "AgnLocusIndex.h"
#include "AgnPairwiseCompareLocus.h"
#include "AgnUtils.h"

//------------------------------------------------------------------------------
// Data structure definition
//------------------------------------------------------------------------------
struct AgnLocusIndex
{
  GtStrArray *seqids;
  GtHashmap *locus_trees;
};


//------------------------------------------------------------------------------
// Prototypes for private methods
//------------------------------------------------------------------------------

/**
 * Given two sets of annotations for the same sequence (a reference set and a
 * prediction set), this function associates each gene annotation with the
 * appropriate locus.
 *
 * @param[in] idx       the locus index
 * @param[in] seqid     the sequence for which loci are requested
 * @param[in] refr      reference annotations for the sequence
 * @param[in] pred      prediction annotations for the sequence
 * @param[in] logger    object to which warning/error messages are written
 * @returns             an interval tree containing the sequence's gene loci
 */
GtIntervalTree *agn_locus_index_parse_pairwise(AgnLocusIndex *idx,
                                               const char *seqid,
                                               GtFeatureIndex *refr,
                                               GtFeatureIndex *pred,
                                               AgnLogger *logger);


//------------------------------------------------------------------------------
// Method implementations
//------------------------------------------------------------------------------

void agn_locus_index_delete(AgnLocusIndex *idx)
{
  gt_hashmap_delete(idx->locus_trees);
  gt_str_array_delete(idx->seqids);
  gt_free(idx);
  idx = NULL;
}

AgnLocusIndex *agn_locus_index_new()
{
  AgnLocusIndex *idx = gt_malloc( sizeof(AgnLocusIndex *) );
  idx->seqids = gt_str_array_new();
  //idx->locus_trees = gt_hashmap_new(GT_HASH_STRING, NULL,
  //                                  (GtFree)gt_interval_tree_delete);
  idx->locus_trees = gt_hashmap_new(GT_HASH_STRING, NULL,
                                    (GtFree)gt_array_delete);
  return idx;
}

GtIntervalTree *agn_locus_index_parse_pairwise(AgnLocusIndex *idx,
                                               const char *seqid,
                                               GtFeatureIndex *refr,
                                               GtFeatureIndex *pred,
                                               AgnLogger *logger)
{
  GtError *error = gt_error_new();
  GtIntervalTree *loci = gt_interval_tree_new(
                             (GtFree)agn_pairwise_compare_locus_delete);

  GtArray *refr_list = gt_feature_index_get_features_for_seqid(refr, seqid,
                                                               error);
  if(gt_error_is_set(error))
  {
    agn_logger_log_error(logger, "error fetching reference features for "
                         "sequence '%s': %s", seqid, gt_error_get(error));
    gt_error_delete(error);
    return NULL;
  }

  GtArray *pred_list = gt_feature_index_get_features_for_seqid(pred, seqid,
                                                               error);
  if(gt_error_is_set(error))
  {
    agn_logger_log_error(logger, "error fetching prediction features for "
                         "sequence '%s': %s", seqid, gt_error_get(error));
    gt_error_delete(error);
    return NULL;
  }

  GtHashmap *procd_genes_refr = gt_hashmap_new(GT_HASH_DIRECT, NULL, NULL);
  GtHashmap *procd_genes_pred = gt_hashmap_new(GT_HASH_DIRECT, NULL, NULL);
  unsigned long num_refr_genes = gt_array_size(refr_list);
  unsigned long num_pred_genes = gt_array_size(pred_list);
  unsigned long i;

  for(i = 0; i < num_refr_genes; i++)
  {
    GtFeatureNode *refr_gene = *(GtFeatureNode**)gt_array_get(refr_list, i);
    if(gt_hashmap_get(procd_genes_refr, refr_gene) == NULL)
    {
      gt_hashmap_add(procd_genes_refr, refr_gene, refr_gene);
      AgnPairwiseCompareLocus *locus = agn_pairwise_compare_locus_new(seqid);
      agn_pairwise_compare_locus_add_refr_gene(locus, refr_gene);

      int new_gene_count = 0;
      do
      {
        int new_refr_gene_count = 0;
        int new_pred_gene_count = 0;

        GtArray *pred_genes_to_add = gt_array_new( sizeof(GtFeatureNode *) );
        GtRange locusrange;
        locusrange.start = agn_pairwise_compare_locus_get_start(locus);
        locusrange.end = agn_pairwise_compare_locus_get_end(locus);
        gt_feature_index_get_features_for_range(pred, pred_genes_to_add, seqid,
                                                &locusrange, error);
        if(gt_error_is_set(error))
        {
          agn_logger_log_error(logger, "error fetching prediction features for "
                               "sequence '%s': %s", seqid, gt_error_get(error));
          gt_error_delete(error);
          return NULL;
        }
        while(gt_array_size(pred_genes_to_add) > 0)
        {
          GtFeatureNode *pred_gene_to_add = *(GtFeatureNode **)
                                                gt_array_pop(pred_genes_to_add);
          if(gt_hashmap_get(procd_genes_pred, pred_gene_to_add) == NULL)
          {
            gt_hashmap_add(procd_genes_pred,pred_gene_to_add,pred_gene_to_add);
            agn_pairwise_compare_locus_add_pred_gene(locus, pred_gene_to_add);
            new_pred_gene_count++;
          }
        }
        gt_array_delete(pred_genes_to_add);

        GtArray *refr_genes_to_add = gt_array_new( sizeof(GtFeatureNode *) );
        locusrange.start = agn_pairwise_compare_locus_get_start(locus);
        locusrange.end = agn_pairwise_compare_locus_get_end(locus);
        gt_feature_index_get_features_for_range(refr, refr_genes_to_add, seqid,
                                                &locusrange, error);
        if(gt_error_is_set(error))
        {
          agn_logger_log_error(logger, "error fetching reference features for "
                               "sequence '%s': %s", seqid, gt_error_get(error));
          gt_error_delete(error);
          return NULL;
        }
        while(gt_array_size(refr_genes_to_add) > 0)
        {
          GtFeatureNode *refr_gene_to_add = *(GtFeatureNode **)
                                                gt_array_pop(refr_genes_to_add);
          if(gt_hashmap_get(procd_genes_refr, refr_gene_to_add) == NULL)
          {
            gt_hashmap_add(procd_genes_refr,refr_gene_to_add,refr_gene_to_add);
            agn_pairwise_compare_locus_add_refr_gene(locus, refr_gene_to_add);
            new_refr_gene_count++;
          }
        }
        gt_array_delete(refr_genes_to_add);
        new_gene_count = new_refr_gene_count + new_pred_gene_count;

      } while(new_gene_count > 0);
      GtIntervalTreeNode *itn = gt_interval_tree_node_new(locus,
                                    agn_pairwise_compare_locus_get_start(locus),
                                    agn_pairwise_compare_locus_get_end(locus));
      gt_interval_tree_insert(loci, itn);
    }
  }

  // FIXME this assumes prediction-unique genes do not overlap
  for(i = 0; i < num_pred_genes; i++)
  {
    GtFeatureNode *pred_gene = *(GtFeatureNode**)gt_array_get(pred_list,i);
    if(gt_hashmap_get(procd_genes_pred, pred_gene) == NULL)
    {
      gt_hashmap_add(procd_genes_pred, pred_gene, pred_gene);
      AgnPairwiseCompareLocus *locus = agn_pairwise_compare_locus_new(seqid);
      agn_pairwise_compare_locus_add_pred_gene(locus, pred_gene);
      GtIntervalTreeNode *itn = gt_interval_tree_node_new(locus,
                                    agn_pairwise_compare_locus_get_start(locus),
                                    agn_pairwise_compare_locus_get_end(locus));
      gt_interval_tree_insert(loci, itn);
    }
  }
  gt_hashmap_delete(procd_genes_refr);
  gt_hashmap_delete(procd_genes_pred);
  gt_array_delete(refr_list);
  gt_array_delete(pred_list);

  return loci;
}

unsigned long agn_locus_index_parse_pairwise_memory(AgnLocusIndex *idx,
                  GtFeatureIndex *refrfeats, GtFeatureIndex *predfeats,
                  int numprocs, AgnLogger *logger)
{
  int i, rank;
  gt_assert(idx != NULL);
  gt_assert(refrfeats != NULL && predfeats != NULL);
  GtStrArray *seqids = agn_seq_intersection(refrfeats, predfeats, logger);
  if(agn_logger_has_error(logger))
  {
    gt_str_array_delete(seqids);
    return 0;
  }
  gt_str_array_delete(idx->seqids);
  idx->seqids = seqids;
  
  unsigned long totalloci = 0;
  #pragma omp parallel private(i, rank)
  {
    rank = omp_get_thread_num();

    #pragma omp for schedule(static)
    for(i = 0; i < gt_str_array_size(seqids); i++)
    {
      const char *seqid = gt_str_array_get(seqids, i);
      GtIntervalTree *loci = agn_locus_index_parse_pairwise(idx, seqid,
                                                            refrfeats,
                                                            predfeats, logger);
      #pragma omp critical
      {
        totalloci += gt_interval_tree_size(loci);
        gt_hashmap_add(idx->locus_trees, (char *)seqid, loci);
        agn_logger_log_status(logger, "loci for sequence '%s' identified by "
                              "processor %d", seqid, rank);
      }
    }
  } // End parallelize
  
  return totalloci;
}

unsigned long agn_locus_index_parse_pairwise_disk(AgnLocusIndex *idx,
                  const char *refrfile, const char *predfile, int numprocs,
                  AgnLogger *logger)
{
  gt_assert(idx != NULL);
  unsigned long nloci;
  GtFeatureIndex *refrfeats = agn_import_canonical(1, &refrfile, logger);
  GtFeatureIndex *predfeats = agn_import_canonical(1, &predfile, logger);
  if(agn_logger_has_error(logger))
  {
    gt_feature_index_delete(refrfeats);
    gt_feature_index_delete(predfeats);
    return 0;
  }
  
  nloci = agn_locus_index_parse_pairwise_memory(idx, refrfeats, predfeats,
                                                numprocs, logger);
  gt_feature_index_delete(refrfeats);
  gt_feature_index_delete(predfeats);
  return nloci;
}

unsigned long agn_locus_index_parse_memory(AgnLocusIndex * idx,
                  GtFeatureIndex *features, int numprocs, AgnLogger *logger)
{
  int i, j, rank;
  gt_assert(idx != NULL && features != NULL);
  GtError *error = gt_error_new();
  GtStrArray *seqids = gt_feature_index_get_seqids(features, error);
  if(gt_error_is_set(error))
  {
    agn_logger_log_error(logger, "error fetching seqids: %s",
                         gt_error_get(error));
    gt_error_delete(error);
    gt_str_array_delete(seqids);
    return 0;
  }
  gt_str_array_delete(idx->seqids);
  idx->seqids = seqids;
  
  unsigned long totalloci = 0;
  #pragma omp parallel private(i, rank)
  {
    rank = omp_get_thread_num();
    
    #pragma omp for schedule(static)
    for(i = 0; i < gt_str_array_size(seqids); i++)
    {
      const char *seqid = gt_str_array_get(seqids, i);
      GtArray *seqfeatures = gt_feature_index_get_features_for_seqid(features,
                                 seqid, error);
      if(gt_error_is_set(error))
      {
        agn_logger_log_error(logger, "error fetching features for seqid '%s': "
                             "%s", seqid, gt_error_get(error));
        gt_error_unset(error);
        //break; // Break not supported in OpenMP for loops
      }
      
      GtArray *loci = gt_array_new( sizeof(AgnLocus *) );
      GtHashmap *genes_visited = gt_hashmap_new(GT_HASH_DIRECT, NULL, NULL);
      for(j = 0; j < gt_array_size(seqfeatures); j++)
      {
        GtFeatureNode *feature = *(GtFeatureNode **)gt_array_get(seqfeatures,j);
        GtFeatureNodeIterator *iter = gt_feature_node_iterator_new(feature);
        GtFeatureNode *fn;
        for(fn = gt_feature_node_iterator_next(iter);
            fn != NULL;
            fn = gt_feature_node_iterator_next(iter))
        {
          if(!gt_feature_node_has_type(fn, "gene") ||
             gt_hashmap_get(genes_visited, fn) != NULL)
          {
            continue;
          }
          
          AgnLocus *locus = agn_locus_new(seqid);
          agn_locus_add(locus, fn);
          
          int new_gene_count = 0;
          do
          {
            int temp_new_gene_count = 0;
            GtArray *overlapping_feats = gt_array_new(sizeof(GtFeatureNode *));
            gt_feature_index_get_features_for_range(features,overlapping_feats,
                                                    seqid,&locus->range,error);
            if(gt_error_is_set(error))
            {
              agn_logger_log_error(logger, "error fetching features for range "
                                   "%s[%lu, %lu]: %s", seqid,
                                   locus->range.start, locus->range.end,
                                   gt_error_get(error));
              gt_error_unset(error);
              //break; // Break not supported in OpenMP for loops; plus, it's
              // the outer loop that I want to break out of anyway
            }
            
            while(gt_array_size(overlapping_feats) > 0)
            {
              GtFeatureNode *fn2add = *(GtFeatureNode **)
                                          gt_array_pop(overlapping_feats);
              if(gt_feature_node_has_type(fn2add, "gene") &&
                 gt_hashmap_get(genes_visited, fn2add) == NULL)
              {
                gt_hashmap_add(genes_visited, fn2add, fn2add);
                agn_locus_add(locus, fn2add);
                temp_new_gene_count++;
              }
            }
            gt_array_delete(overlapping_feats);
            new_gene_count = temp_new_gene_count;
          } while(new_gene_count > 0);
          
          gt_array_add(loci, locus);
        }
      }
      gt_hashmap_delete(genes_visited);
      
      #pragma omp critical
      {
        totalloci += gt_array_size(loci);
        gt_hashmap_add(idx->locus_trees, (char *)seqid, loci);
        agn_logger_log_status(logger, "loci for sequence '%s' identified by "
                              "processor %d", seqid, rank);
      }
    } // End parallelize
  }
  
  gt_error_delete(error);
  return totalloci;
}

unsigned long agn_locus_index_parse_disk(AgnLocusIndex * idx, int numfiles,
                  const char **filenames, int numprocs, AgnLogger *logger)
{
  gt_assert(idx != NULL);
  unsigned long nloci;
  // Do I want to use the import canonical function or the add_gff3 function?
  GtFeatureIndex *features = agn_import_canonical(numfiles, filenames, logger);
  if(agn_logger_has_error(logger))
  {
    gt_feature_index_delete(features);
    return 0;
  }
  
  nloci = agn_locus_index_parse_memory(idx, features, numprocs, logger);
  gt_feature_index_delete(features);
  return nloci;
}