// -----------------------------------------------------------------------------------------------
#ifndef _DEFS_H
#define _DEFS_H
// -----------------------------------------------------------------------------------------------

// command line parameters
#define CMD_LINE_ARG_HIST                           'H'
#define CMD_LINE_ARG_VERTEX_SAMPLE                  'V'
#define CMD_LINE_ARG_EDGE_SAMPLE                    'E'
#define CMD_LINE_ARG_FACE_SAMPLE                    'F'
#define CMD_LINE_ARG_SAMPLE_TYPE                    'S'
#define CMD_LINE_ARG_MONTECARLO_SAMPLING            'M'
#define CMD_LINE_ARG_SUBDIVISION_SAMPLING           'S'
#define CMD_LINE_ARG_SIMILAR_TRIANGLES_SAMPLING     'T'
#define CMD_LINE_ARG_N_SAMPLES                      'N'
#define CMD_LINE_ARG_SAMPLES_PER_AREA_UNIT          'A'
#define CMD_LINE_ARG_SAVE_DISPLACEMENT              'O'
#define CMD_LINE_ARG_SAVE_ERROR_AS_COLOUR           'C'
#define CMR_LINE_ARG_INCLUDE_UNREF										'U'

// error messages
#define MSG_ERR_N_ARGS                  "\nUsage:  "\
                                        "metro file1 file2  [opt]\n"\
                                        "where opt can be:\n"\
                                        "-V         disable vertex sampling\n"\
                                        "-U         ignore unreferred vertices\n"\
                                        "-E         disable edge sampling\n"\
                                        "-F         disable face sampling\n"\
                                        "-Sx        set the face sampling mode\n"\
                                        "           where x can be:\n"\
                                        "            -M  montecarlo sampling\n"\
                                        "            -S  subdivision sampling\n"\
                                        "            -T  similar triangles sampling\n"\
                                        "-N#        set the required number of samples (overrides -A)\n"\
                                        "-A#        set the required number of samples per area unit (overrides -N)\n"\
                                        "-Hxxx.yyy  save histogram of error values in the file xxx.yyy\n"\
                                        "-O         save error as vertex quality\n"\
                                        "-C         save error as vertex colour"\
                                        "\n"

#define MSG_ERR_MESH_LOAD               "error loading the input meshes.\n"
#define MSG_ERR_INVALID_OPTION          "unable to parse option '%s'\n"
#define MSG_ERR_FILE_OPEN               "unable to open the output file.'n"
#define MSG_ERR_UNKNOWN_FORMAT          "unknown file format '%s'.\n"

// global constants
#define NO_SAMPLES_PER_FACE             10
#define N_SAMPLES_EDGE_TO_FACE_RATIO    0.1
#define BBOX_FACTOR                     0.1
#define INFLATE_PERCENTAGE			    0.02
#define MIN_SIZE					    125		/* 125 = 5^3 */
#define N_HIST_BINS                     256
#define PRINT_EVERY_N_ELEMENTS          1000
#define FILE_EXT_SMF                    "smf"
#define FILE_EXT_PLY                    "ply"

// strings
#define STR_HIST_FILENAME_DEFAULT       "hist.txt"
#define STR_NEW_MESH_FILENAME_DEFAULT   "error.ply"
#define STR_NEW_MESH_FILENAME_DEFAULT_2 "error_colour.ply"

// -----------------------------------------------------------------------------------------------
#endif
// -----------------------------------------------------------------------------------------------
