// originally, these were code snippets in hdate main()
// now want to make common with hcal

// !! FIXME - currently requires definition of opt.quiet_alert


#include <fnmatch.h>	// For fnmatch

// support for fnmatch
#define FNM_EXTMATCH	(1 << 5)
#define FNM_NOFLAG		0
#define NEGATIVE_NUMBER_GLOB	"-*([[:digit:]])?(.*([[:digit:]]))"
#define TIMEZONE_GLOB			"?([+-])?(1)[[:digit:]]"
#define COORDINATE_GLOB			"?([+-])[[:digit:]]?([[:digit:]])?(.+([[:digit:]]))"

#define BAD_COORDINATE	999
#define BAD_TIMEZONE	999
#define DELTA_LONGITUDE 30

#define EXIT_CODE_BAD_PARMS	1

double lat = BAD_COORDINATE;	/* set to this value for error handling */
double lon = BAD_COORDINATE;	/* set to this value for error handling */
int tz = BAD_TIMEZONE;			/* -z option Time Zone, default to system local time */

int opt_latitude = 0;			/* -l option latitude */
int opt_Longitude = 0;			/* -L option longitude */

int error_detected = FALSE;		/* exit after reporting ALL bad parms */


// for function hdate_validate
#define CHECK_DAY_PARM   1
#define CHECK_MONTH_PARM 2
#define CHECK_YEAR_PARM  3


/************************************************************
* begin - error message functions
************************************************************/
void print_parm_error ( char *parm_name )
{
	error(0,0,"%s: %s %s %s",N_("error"), N_("parameter to option"), parm_name, N_("is non-numeric or out of bounds"));
}

void print_parm_missing_error ( char *parm_name )
{
	error(0,0,"%s: %s %s %s",N_("error"), N_("option"), parm_name, N_("missing parameter"));
}

void print_alert_timezone( int tz )
{
	error(0,0,"%s: %s, %+d UTC",N_("ALERT: time zone not entered, using system local time zone"), *tzname, tz);
}
/************************************************************
* end - error message functions
************************************************************/



void print_alert_coordinate( char *city_name )
{
	error(0,0,"%s %s", N_("ALERT: guessing... will use co-ordinates for"), city_name);
}

void print_alert_default_location( int tz )
{
	switch (tz)
	{
	case -8:	print_alert_coordinate( N_("Los Angeles") ); break;
	case -6:	print_alert_coordinate( N_("Mexico City") ); break;
	case -5:	print_alert_coordinate( N_("New York City") ); break;
//	case -5:	print_alert_coordinate( N_("Toronto") ); break;
//	case -3:	print_alert_coordinate( N_("Sao Paolo") ); break;
	case -3:	print_alert_coordinate( N_("Buenos Aires") ); break;
	case  0:	print_alert_coordinate( N_("London") ); break;
	case  1:	print_alert_coordinate( N_("Paris") ); break;
	case  2:	print_alert_coordinate( N_("Tel-Aviv") ); break;
	case  3:	print_alert_coordinate( N_("Moscow") ); break;
	case  5:	print_alert_coordinate( N_("Tashkent") ); break;
	case  8:	print_alert_coordinate( N_("Beijing") ); break;
//	case  8:	print_alert_coordinate( N_("Hong Kong") ); break;
	case -10:	print_alert_coordinate( N_("Honolulu") ); break;
	default:	error(0,0,"%s \n%s", N_("Hmmm, ... hate to do this, really ..."), N_("using co-ordinates for the equator, at the center of time zone")); break;
	}
}


/************************************************************
* set default location
************************************************************/
void set_default_location( int tz, double *lat, double *lon )
{
	/*	Temporarily, set some default lat/lon coordinates
		for certain timezones */
	switch (tz)
	{
	case -8:	*lat =  34.05;	*lon =-118.25;	break; // Los Angeles
	case -6:	*lat =  19.43;	*lon = -99.13;	break; // Mexico City
	case -5:	*lat =  40.0;	*lon = -74.0;	break; // New York
//	case -5:	*lat = -43.71;	*lon = -79.43;	break; // Toronto
//	case -3:	*lat = -23.55;	*lon = -46.61;	break; // Sao Paolo
	case -3:	*lat = -34.6;	*lon = -58.37;	break; // Buenos Aires
	case  0:	*lat =  51.0;	*lon =   0.0;	break; // London
	case  1:	*lat =  48.0;	*lon =   2.0;	break; // Paris
	case  2:	*lat =  32.0;	*lon =  34.0;	break; // Tel aviv
	case  3:	*lat =  55.0;	*lon =  37.0;	break; // Moscow
	case  5:	*lat =  41.27;	*lon =  69.22;	break; // Tashkent
	case  8:	*lat =  39.90;	*lon = 116.38;	break; // Beijing
//	case  8:	*lat =  22.26;	*lon = 114.15;	break; // Hong Kong
	case 10:	*lat =  21.30;	*lon = 157.82;	break; // Honolulu
	default:	*lat =   0.0;	*lon =  tz * 15; break; /* center of tz */
	}
	return;
}


/************************************************************
* check for sunset
************************************************************/
int check_for_sunset (hdate_struct * h, double lat, double lon, int timezone )
{
	time_t now_time;
	struct tm *now_timep;
	int sunrise, sunset;

	hdate_get_utc_sun_time (h->gd_day, h->gd_mon, h->gd_year, lat, lon,
							&sunrise, &sunset);

	time(&now_time);
	now_timep = localtime(&now_time);

	if ( ((now_timep->tm_hour) *60 + now_timep->tm_min) > (sunset + (timezone*60) ) ) return 1;
	else return 0;
}



/************************************************************
* parse latitude
************************************************************/
void parse_latitude()
{
	if	((!optarg) ||
		 (	(fnmatch("-*",optarg,FNM_NOFLAG)==0) &&
			(fnmatch(NEGATIVE_NUMBER_GLOB,optarg,FNM_EXTMATCH)!=0)) )
	{
		print_parm_missing_error(N_("l"));
		error_detected = TRUE;
		if (optarg) optind--;
	}
	else
	{
		lat = (double) atof (optarg);
		if	((fnmatch(COORDINATE_GLOB,optarg,FNM_EXTMATCH)==0) &&
			 (lat > -90) && (lat < 90))
		{
			opt_latitude = 1;
		}
		else
		{
			print_parm_error(N_("l (latitude)"));
			error_detected = TRUE;
		}
	}
}


/************************************************************
* parse longitude
************************************************************/
void parse_longitude()
{
	if	((!optarg) ||
		 (	(fnmatch("-*",optarg,FNM_NOFLAG)==0) &&
			(fnmatch(NEGATIVE_NUMBER_GLOB,optarg,FNM_EXTMATCH)!=0)) )
	{
		print_parm_missing_error(N_("L"));
		error_detected = TRUE;
		if (optarg) optind--;
	}
	else
	{
		lon = (double) atof (optarg);
		if	((fnmatch(COORDINATE_GLOB,optarg,FNM_EXTMATCH)==0) &&
			 (lon > -180) && (lon < 180))
		{
			opt_Longitude = 1;
		}
		else
		{
			print_parm_error(N_("L (longitude)"));
			error_detected = TRUE;
		}
	}
}

/************************************************************
* parse timezone
************************************************************/
void parse_timezone()
{
	if	((!optarg) ||
		 (	(fnmatch("-*",optarg,FNM_NOFLAG)==0) &&
			(fnmatch(NEGATIVE_NUMBER_GLOB,optarg,FNM_EXTMATCH)!=0)) )
	{
		print_parm_missing_error(N_("z"));
		error_detected = TRUE;
		if (optarg) optind--;
	}
	else
	{
		tz = atoi (optarg);
		if	((fnmatch(TIMEZONE_GLOB,optarg,FNM_EXTMATCH)!=0) ||
			 (tz < -12) || (tz > 12))
		{
			print_parm_error(N_("z (time zone)"));
			error_detected = TRUE;
		}
	}
}


/************************************************************
* validate location parameters - lat, lon, tz
************************************************************/
// originally, this was only for hdate, now also for hcal
// originally, we did this only if we needed the information
// now, because of sunset-awareness, we always perform these checks
// if ( (opt.sun) || (opt.times) || (opt.candles) || (opt.havdalah) )
void validate_location()
{

	/* latitude and longitude must be paired */
	if ( (opt_latitude) && (!opt_Longitude) )
	{
		error(0,0,"%s: %s", N_("error"), N_("valid longitude parameter missing for given latitude"));
		error_detected = TRUE;
	}
	else if ( (opt_Longitude) && (!opt_latitude) )
	{
		error(0,0,"%s: %s", N_("error"), N_("valid latitude parameter missing for given longitude"));
		error_detected = TRUE;
	}


	if (!error_detected)
	{
		/* if no timezone entered, choose guess method */
		if (tz==BAD_TIMEZONE)
		{
			tzset();
			tz = timezone /-3600;
			if ( (opt_latitude) && (opt_Longitude) && ( ((lon/15)-tz) > DELTA_LONGITUDE ) )
			{
				/*  reject system local timezone, because it's
					too far from the longitude explicitly provided
					by the user; guess based on longitude entered */
				tz = lon /15;
			}
			if (!opt.quiet_alerts) print_alert_timezone( tz );
			if ( (lat==BAD_COORDINATE) && (lon==BAD_COORDINATE) )
			{
				if (!opt.quiet_alerts) print_alert_default_location( tz );
				set_default_location( tz, &lat, &lon );
			}
			printf("\n");
		}
		else
		{	/* we have timezone, what about latitude and longitude? */
			if ( (opt_Longitude) && (opt_latitude) )
			{
				/* sanity-check longitude versus timezone */
				if ( abs( ( tz * 15 ) - lon ) > DELTA_LONGITUDE )
				{
					error(0,0,"%s %d %s %.3f %s", N_("time zone value of"), tz, N_("is incompatible with a longitude of"), lon, N_("degrees"));
					error_detected = TRUE;
				}
			}
			else
			{
				if (!opt.quiet_alerts) print_alert_default_location( tz );
				set_default_location( tz, &lat, &lon );
				printf("\n");
			}
		}
	}

	/* exit after reporting all bad parameters found */
	if (error_detected)
	{
		print_usage();
		exit(EXIT_CODE_BAD_PARMS);
	}
}



/************************************************************
* check validity of date parameters
************************************************************/
int hdate_validate (int parameter_to_check, int day, int month, int year)
{
	hdate_struct h;
	
	if (year < 0) return FALSE;
	
	switch (parameter_to_check)
	{

	case CHECK_DAY_PARM:

		/************************************************************
		* check day in Hebrew date
		************************************************************/
		if (year > 3000)
		{
			hdate_set_hdate (&h, 1, 1, year);
			if ((day < 1) || (day > 30) ||
				((day > 29) && ((month==4) || (month==6) || (month==8) || (month=10) || (month==12) || (month==14))) ||
				(((h.hd_size_of_year==353) || (h.hd_size_of_year==383)) && ((month==2) || (month==3)) && (day >29)) ||
				(((h.hd_size_of_year==354) || (h.hd_size_of_year==384)) && (month==2) && (day>29)) )			
				return FALSE;
			return TRUE;
		}

		/************************************************************
		* check day in Gregorian date
		************************************************************/
		else
		{
			if ((day < 1) || 
				((day > 31) && ((month==1) || (month==3) || (month==5) || (month==7) || (month==8) || (month==10) ||(month==12))) ||
				((day > 30) && ((month==4) || (month==6) || (month==9) || (month==11))) ||
				((day > 29) && (month==2) && ((year%4)==0)) ||
				((day > 28) && (month==2) && ((year%4)!=0))
			   )
				return FALSE;
			return TRUE;
		}
		break;	


	case CHECK_MONTH_PARM:

		/************************************************************
		* check month in Hebrew date
		************************************************************/
		if (year > 3000)
		{
			if ((month <= 0) || (month > 14)) return FALSE;
			hdate_set_hdate (&h, 1, 1, year);
			if ((h.hd_size_of_year <365) && (month >12)) return FALSE;
			return TRUE;
		}

		/************************************************************
		* check month in Gregorian date
		************************************************************/
		else
		{
			if ((month <= 0) || (month > 12)) return FALSE;
			return TRUE;
		}
		break;
	
	case CHECK_YEAR_PARM:
		return TRUE;
		break;
	}
	return FALSE;
}

