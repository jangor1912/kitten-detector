//
// Created by jan on 07.06.22.
//

#define MAX_SOURCE_URI_LENGTH 1024

#include <stdio.h>
#include <stdlib.h>

#include <pipeline.h>

/* Program takes following args:
 * inference-width
 * inference-height
 * max-batch-size
 * source-uri-1
 * source-uri-2
 * ...
 * source-uri-n
 * */
int main(int argc, char *argv[])
{
    if(argc < 5){
        printf("Expecting at least 4 arguments: "
               "<inference-width> <inference-height> <max-batch-size> <source-uri>\n");
        return 1;
    }

    printf("Got %d arguments:\n", argc);
    for(int i=0; i<argc; i++){
        printf("\tArgument %d = %s\n", i, argv[i]);
    }

    int width = atoi(argv[1]);
    int height = atoi(argv[2]);
    int max_batch_size = atoi(argv[3]);
    int sources_number = argc - 4;

    char **source_uris = (char **) malloc(sizeof(char *) * sources_number);
    int current_source_number = 0;
    for(int argument_number = 4; argument_number < argc; argument_number++){
        source_uris[current_source_number] = argv[argument_number];
        current_source_number++;
    }

    StreamMuxerConfig *streammux_config = (StreamMuxerConfig *) malloc(sizeof(StreamMuxerConfig));
    streammux_config->width = width;
    streammux_config->height = height;
    streammux_config->max_batch_size = max_batch_size;

    SourcesConfig *sources_config = (SourcesConfig *) malloc(sizeof(SourcesConfig));
    sources_config->sources_number = sources_number;
    sources_config->source_uris = source_uris;

    printf("Successfully allocated pipeline-initialisation structures. Running pipeline!\n");
    int result = run_pipeline(sources_config, streammux_config);

    if(result == 0){
        printf("Pipeline has finished running successfully!\n");
    } else {
        printf("Error during running the pipeline!\n");
    }

    free(source_uris);
    free(streammux_config);
    free(sources_config);

    return result;
}
