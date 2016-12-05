/*
 * wind.cpp

 *
 *  Created on: Sep 16, 2016
 *      Author: fiala
 */

#include "wind.h"
#include "fc.h"
#include "../common.h"

//#include <math.h>

//tolerances
#define badPointsTol 	2
#define angleTol 		25 	// deg
//averaging system settings
#define avg_weight_max	100
#define avg_weight_min	1
#define avg_weight_inc	1
#define avg_weight_dec	1

/// for testing purposes
//#define wind_debug_enable
#ifdef wind_debug
void debug_kBuffer(int);
struct wind_debug
{
	int pointNumber;

};
wind_debug windDbg;
#endif
///

float my_atan2(float x, float y)
{	// calculates atan2, returns: angle in deg, range < 0 ; 360 ), max error 0.162 deg

//	DEBUG("my_atan %ld %ld\n", x, y);

	if(x < 0 and y == 0)
    {
    	return 270.0;
    }
    if(x > 0 and y == 0)
    {
    	return 90.0;
    }
    if(x == 0 and y == 0)
    {
    	return (-1.0);//error condition
    }
    if(x == 0 and y > 0)
    {
    	return 0.0;
    }
    if(x == 0 and y < 0)
    {
    	return 180.0;
    }

    ///arctan aproximation
    float fi = fabs( x / y );
//    DEBUG("x/y= %f ",fi);
    float fi2 = fi * fi;
//    DEBUG("(x/y)^2= %f ",fi2);
    fi = ((0.596227 * fi + fi2) / (1 + 2 * 0.596227 * fi + fi2 )) * 90;
//    DEBUG("fi= %f ",fi);

    /// cover all quadrants
    if(x >= 0 and y > 0)
	{
    	return (fi);
	}
    if(x < 0 and y > 0)
    {
    	return (360.0 - fi);
    }
    if(x >= 0 and y < 0)
    {
    	return (180.0 - fi);
    }
    if(x < 0 and y < 0)
    {
    	return (fi + 180.0);
    }
/*    if(x < 0 and y == 0)
    {
    	return (fi - 180.0);
    }
 */
    ///
    return (-1.0); //error condition
}

void wind_kBuff_init()
{
	//fc.wind.kBuff_start = 0;
	fc.wind.kBuff_end   = 0;
}

void wind_kBuff_write(float X, float Y)
{
	fc.wind.kBuff[fc.wind.kBuff_end].X = X;
	fc.wind.kBuff[fc.wind.kBuff_end].Y = Y;
	fc.wind.kBuff_end = (fc.wind.kBuff_end + 1) % wind_kBuff_size;	//any size
	//fc.wind.kBuff_end = (fc.wind.kBuff_end + 1) & (wind_kBuff_size - 1);	//buff size must be: 2^n (2,4,8,16,32,...)
}
void wind_kBuff_read(int i, bool reversed, float * x, float * y)
{
	if(reversed) //read form end
	{
		*x = fc.wind.kBuff[(wind_kBuff_size - i + fc.wind.kBuff_end - 1) % wind_kBuff_size].X;	//any size
		*y = fc.wind.kBuff[(wind_kBuff_size - i + fc.wind.kBuff_end - 1) % wind_kBuff_size].Y;	//any size
	}
	else		//read from begining
	{
		*x = fc.wind.kBuff[(i + fc.wind.kBuff_end - 1) % wind_kBuff_size].X;	//any size
		*y = fc.wind.kBuff[(i + fc.wind.kBuff_end - 1) % wind_kBuff_size].Y;	//any size
	}
}

void wind_init()
{
	wind_kBuff_init();
	fc.wind.old_gps.X = 0;
	fc.wind.old_gps.Y = 0;
	fc.wind.fromEnd = true;
	fc.wind.kBuff_full = false;
	fc.wind.first_wind_calc_avg = true;
	fc.wind.first_wind_calc_data = true;
	fc.wind.valid = false;
#ifdef wind_debug_enable
	windDbg.pointNumber = 0;
#endif
	DEBUG("wind_init\n");

}

void wind_new_gps_fix()  //calculte dX,dY from gps position, save to kBuff, calculate wind if possible
{
	//DEBUG("Wind calc start\n");
#ifdef wind_debug_enable
	windDbg.pointNumber += 1;
#endif
	if(fc.wind.first_wind_calc_data == true)   //first time run
	{
		fc.wind.old_gps.X = float(fc.gps_data.longtitude) / 1.0 ;
		fc.wind.old_gps.Y = float(fc.gps_data.latitude) / 1.0  ;
		fc.wind.first_wind_calc_data = false;
		return;
	}

	//uint32_t dTime = fc.gps_data.utc_time - fc.wind.old_time;
	float currLon = float(fc.gps_data.longtitude) / 1.0;
	float currLat = float(fc.gps_data.latitude)   / 1.0;
	float dX = (currLon - fc.wind.old_gps.X);	// / dTime;
	float dY = (currLat  - fc.wind.old_gps.Y);	// / dTime;

	fc.wind.old_time = fc.gps_data.utc_time;
	fc.wind.old_gps.X = currLon;
	fc.wind.old_gps.Y = currLat;

	//DEBUG("coords: %f,%f\n", currLon, currLat);
	//DEBUG("dX= %f dY= %f \n", dX, dY);
	wind_kBuff_write(dX, dY);

	if(fc.wind.kBuff_end != 0 and fc.wind.kBuff_full == false )	//if buffer is not full
	{
		//DEBUG("Wind calc need more points\n");
	#ifdef wind_debug_enable
		DEBUG("%d 0 %d %d %f %f\n", windDbg.pointNumber, fc.wind.circle_end, fc.wind.fromEnd, currLon, currLat);
	#endif
		return;	//wait for more points
	}
	fc.wind.kBuff_full = true;

	bool first_read = true;
	float sumAngle = 0; 	// used to check if circle happend :)
	uint8_t nOfTurns   = 0;
	uint8_t badPoints  = 0;
	fc.wind.circle_end = 0;

	for (int i=0; i < wind_kBuff_size; i++)
	{
		float kBuffVal_x;
		float kBuffVal_y;

		wind_kBuff_read(i, fc.wind.fromEnd, &kBuffVal_x, &kBuffVal_y);
		//DEBUG("kBuff: %f,%f",kBuffVal_x,kBuffVal_y);
		if(first_read)	//first run
		{
			fc.wind.oldAngle = my_atan2(kBuffVal_x, kBuffVal_y);
			first_read = false;
			continue;
		}

		//DEBUG("kbuff: %f %f", kBuffVal_x, kBuffVal_y); // testing

		float angle = my_atan2( kBuffVal_x , kBuffVal_y );
		float deltaAngle = angle - fc.wind.oldAngle;
        if(deltaAngle >  180)
            deltaAngle -= 360;
        if(deltaAngle < -180)
            deltaAngle += 360;
		fc.wind.oldAngle = angle;
		//DEBUG("points: %f,%f  angle: %f\n",kBuffVal_x, kBuffVal_y, angle); //testing


        if((   (deltaAngle >= 0 and sumAngle >= 0 )
           or  (deltaAngle <= 0 and sumAngle <= 0 )
           or  (sumAngle == 0 or fabs(deltaAngle) < angleTol ))
           and (badPoints <= badPointsTol))
        {
        	if(( -angleTol <= deltaAngle and deltaAngle <= 0) and sumAngle >= 0)
        		badPoints++;
        	else if(( 0 <= deltaAngle and deltaAngle <= angleTol) and sumAngle <= 0)
                badPoints++;
        	else
        		badPoints = 0;

        	//DEBUG("deltaAngle = %f sumAngle= %f\n", deltaAngle, sumAngle); //testing
        	sumAngle = sumAngle + deltaAngle;


        	if ( fabs(sumAngle) >= ( float( (nOfTurns + 1) * 360 ) ) )
            {
                nOfTurns++;
                fc.wind.circle_end = i;
            }
        }
        else
        {
        	sumAngle  = 0;
        	badPoints = 0;
        }
        //DEBUG("sumAngle= %f  n of turns: %d\n",sumAngle,nOfTurns);

	}

	//DEBUG("pocet celych otacok: %d", nOfTurns);
	//DEBUG(" circle end: %d\n",fc.wind.circle_end);

	if(nOfTurns != 0)
	{
		/// kasaFit // circular regression
		//DEBUG("Regresion\n");
	    vector_2d_t points_avg;
	    points_avg.X = 0.0;
	    points_avg.Y = 0.0;


	    for(int i=0; i < fc.wind.circle_end; i++ )	//calculate average coordinates (centroid)
	    {
	    	float kBuffVal_x;
	    	float kBuffVal_y;

	    	wind_kBuff_read(i, fc.wind.fromEnd, &kBuffVal_x, &kBuffVal_y);

	    	points_avg.X += kBuffVal_x;
	    	points_avg.Y += kBuffVal_y;
	    }
	    points_avg.X /= (float) fc.wind.circle_end;
	    points_avg.Y /= (float) fc.wind.circle_end;

	    //     circular regresion

	    float Xi, Yi, Zi;
	    float Mxy, Mxx, Myy, Mxz, Myz;
	    float B, C, G11, G12, G22, D1, D2;
	    Mxx= Myy= Mxy= Mxz= Myz= 0.0;

	    for(int i=0; i < fc.wind.circle_end; i++)
	    {
	    	//vector_2d_t kBuffVal = wind_kBuff_read(i, fc.wind.fromEnd);
	    	float kBuffVal_x;
	    	float kBuffVal_y;
	    	wind_kBuff_read(i, fc.wind.fromEnd, &kBuffVal_x, &kBuffVal_y);
	    	//DEBUG("%f %f\n", kBuffVal_x, kBuffVal_y);

	    	Xi = kBuffVal_x - points_avg.X ;   //  centered x-coordinates
   	        Yi = kBuffVal_y - points_avg.Y ;   //  centered y-coordinates
   	        Zi = Xi*Xi + Yi*Yi;

   	        Mxx += Xi * Xi;
	    	Myy += Yi * Yi;
   	        Mxy += Xi * Yi;
   	        Mxz += Xi * Zi;
   	        Myz += Yi * Zi;
	    }

	    Mxx /= (float) fc.wind.circle_end;
	    Myy /= (float) fc.wind.circle_end;
	    Mxy /= (float) fc.wind.circle_end;
	    Mxz /= (float) fc.wind.circle_end;
	    Myz /= (float) fc.wind.circle_end;

	    //    solving system of equations by Cholesky factorization

        G11 = sqrt(Mxx);
        G12 = Mxy / G11;
        G22 = sqrt(Myy - G12*G12);

        D1 = Mxz / G11;
        D2 = (Myz - D1 * G12) / G22;

	    //    computing paramters of the fitting circle

        C = D2 / G22 / 2.0;
        B = (D1 - G12 * C) / G11 / 2.0;

	    //       assembling the output

        float wind_calcX = B + points_avg.X;
        float wind_calcY = C + points_avg.Y;
        /// end of kasaFit //
        //DEBUG("circ xy: %f %f\n", wind_calcX, wind_calcY);

        if(fc.wind.avg_weight < avg_weight_max)	//change weight of previous measurements
        {
        	fc.wind.avg_weight += avg_weight_inc;
        }
        else
        {
        	fc.wind.avg_weight = avg_weight_max;
        }

        if(fc.wind.first_wind_calc_avg)		//add new calculation to average
        {
        	fc.wind.wind_coords.X = wind_calcX;
        	fc.wind.wind_coords.Y = wind_calcY;
        	fc.wind.valid = true;
        	fc.wind.first_wind_calc_avg = false;
        }
        else
        {
        	fc.wind.wind_coords.X = (fc.wind.wind_coords.X * float(fc.wind.avg_weight) - fc.wind.wind_coords.X + wind_calcX) / float(fc.wind.avg_weight);
          	fc.wind.wind_coords.Y = (fc.wind.wind_coords.Y * float(fc.wind.avg_weight) - fc.wind.wind_coords.Y + wind_calcY) / float(fc.wind.avg_weight);
        }
        																					// * M_PI / 180 * 6378000 / 10000000
        fc.wind.speed = sqrt( pow( fc.wind.wind_coords.X, 2) + pow( fc.wind.wind_coords.Y, 2)) / 89.99335 ;
        fc.wind.direction =  my_atan2(fc.wind.wind_coords.X, fc.wind.wind_coords.Y) + 180.0;
        if (fc.wind.direction > 360)
        	fc.wind.direction -= 360;

        #ifdef wind_debug_enable
        float spd_act = sqrt( wind_calcX * wind_calcX + wind_calcY * wind_calcY ) * M_PI / 180 * 6378000;
        float dir_act =  my_atan2(wind_calcX, wind_calcY);


        DEBUG("%d %d %d %d %f %f %f %f %f %f %f %f", windDbg.pointNumber, nOfTurns, fc.wind.circle_end, fc.wind.fromEnd, currLon, currLat, fc.wind.speed, fc.wind.direction, spd_act, dir_act, wind_calcX, wind_calcY );
        //debug_kBuffer(fc.wind.circle_end);
        DEBUG("\n");
		#endif

	}
	else // (nOfTurns equals 0)
	{
        if(fc.wind.avg_weight > avg_weight_min)	//change weight of previous measurements
        {
        	fc.wind.avg_weight -= avg_weight_dec;
        }
        else
        {
        	fc.wind.avg_weight = avg_weight_min;
        }
	#ifdef wind_debug_enable
    DEBUG("%d %d %d %d %f %f %f %f", windDbg.pointNumber, nOfTurns, fc.wind.circle_end, fc.wind.fromEnd, currLon, currLat, fc.wind.speed, fc.wind.direction);
    //debug_kBuffer(wind_kBuff_size);
    DEBUG("\n");
	#endif

	}

    // set direction of next kBuff read
    if (fc.wind.nOfTurns_old == 0 and nOfTurns == 1) fc.wind.fromEnd = true;
    if (fc.wind.nOfTurns_old == 1 and nOfTurns == 2) fc.wind.fromEnd = false;
    if (fc.wind.nOfTurns_old == 2 and nOfTurns == 1) fc.wind.fromEnd = false;
    if (fc.wind.nOfTurns_old == 1 and nOfTurns == 0) fc.wind.fromEnd = true;
    fc.wind.nOfTurns_old = nOfTurns;



	//DEBUG("Wind calc end\n"); //testing

}

void wind_step()
{
	if (fc.gps_data.new_sample & FC_GPS_NEW_SAMPLE_WIND)
	{
		//uint16_t wind_run_time = debug_timer.GetValue();

		wind_new_gps_fix();

		//wind_run_time = debug_timer.GetValue() - wind_run_time;
		//DEBUG("Wind_run_time: %d\n", wind_run_time);

		fc.gps_data.new_sample &= ~FC_GPS_NEW_SAMPLE_WIND;
	}
}
#ifdef wind_debug_enable
void debug_kBuffer(int numberOfPoints)
{
	float kBuffVal_x;
	float kBuffVal_y;

	DEBUG(" kBuffXY ");
	for(int i=0; i < numberOfPoints; i++ )
	{
		wind_kBuff_read(i, fc.wind.fromEnd, &kBuffVal_x, &kBuffVal_y);
		DEBUG(" %f %f",kBuffVal_x,kBuffVal_y);
	}
}
#endif
