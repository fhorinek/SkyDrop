#ifndef KML_H_
#define KML_H_

#include "../../common.h"

void kml_comment(char * text);
uint8_t kml_start(char * path);
void kml_step();
void kml_stop();


#endif /* KML_H_ */
