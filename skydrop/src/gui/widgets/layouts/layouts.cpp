#include "layouts.h"

#include "1.inc"
#include "11.inc"
#include "113.inc"
#include "12.inc"
#include "121.inc"
#include "122.inc"
#include "123.inc"
#include "132.inc"
#include "133.inc"
#include "21.inc"
#include "22.inc"
#include "222.inc"
#include "223.inc"
#include "233.inc"
#include "333.inc"

//need to be in one line since ee_mapper is lazy parser
const layout_desc * layout_list[NUMBER_OF_LAYOUTS] =
{ &layout_1, &layout_11, &layout_113, &layout_12, &layout_121, &layout_122, &layout_123, &layout_132, &layout_133, &layout_21, &layout_22, &layout_222, &layout_223, &layout_233, &layout_333 };
