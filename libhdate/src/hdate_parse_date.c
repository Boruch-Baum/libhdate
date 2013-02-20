/** hdate_parse_date.c            http://libhdate.sourceforge.net
 *  flexibly parse what could be a Hebrew or gregorian date
 *
 * compile:
 * gcc `pkg-config --libs --cflags libhdate` local_functions.c -o local_functions
 *
 * Copyright:  2013 (c) Boruch Baum
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
//gcc -Wall -c -g -I "../../src" "%f"
//gcc -Wall -g -I "../../src" -L"../../src/.libs" -lhdate -efence -o "%e" "%f"

#include <hdate.h>		/// For hebrew date (gcc -I ../../src)
#include <support.h>	/// libhdate general macros, including for gettext
#include <stdlib.h>		/// For atoi
#include <stdio.h>		/// For printf
#include <error.h>		/// For error
#include <string.h>		/// For strspn, strlen

#define YR_LOWER_4_BOUND 1000
#define YR_UPPER_2_BOUND 99
#define HMONTH_UPPER_BOUND 14
#define GMONTH_UPPER_BOUND 12
#define GDAY_UPPER_BOUND 31
#define HDAY_UPPER_BOUND 30
#define UNKNOWN_STATE 0
#define MUST_BE_YEAR 1
#define MUST_BE_MONTH 10

#define HDATE_PREFER_YM 1
#define HDATE_PREFER_MD 0

typedef struct {
	int a_val;
	int b_val;
	int c_val;
	int a_id;
	int b_id;
	int c_id;	
	int* ret_year;
	int* ret_month;
	int* ret_day;
	int prefer_hebrew;
	int prefer_2_parm_ym;
	int base_year_h;
	int base_year_g;
	} parse_date_struct;
	
static char * day_text		 = N_("day");
static char * month_text	 = N_("month");
static char * year_text 	 = N_("year");
static char * error_text     = N_("error");

void print_parm_error ( const char *parm_name )
{
	error(0,0,"%s: %s %s %s",error_text,
			N_("parameter"), parm_name,
			N_("is non-numeric or out of bounds"));
}

void print_parm_mismatch_error()
{
	error(0,0,"%s: %s",error_text,
			N_("month and year parameters mismatched (Hebrew/gregorian mix)"));
}

/****************************************************
* initial_parse
***************************************************/
int initial_parse( const char* parm_str, int* parm_val, int* parm_id, parse_date_struct* p )
{
	const char* digits = "0123456789";
	int parm_span, parm_len;

	parm_span = strspn(parm_str, digits);
	parm_len  = strlen(parm_str);
	if (parm_len == parm_span) /// numeric parm
	{
		*parm_val = atoi(parm_str);
		if (   (*parm_val > HEB_YR_UPPER_BOUND) ||
			 ( (*parm_val < YR_LOWER_4_BOUND) && (*parm_val > YR_UPPER_2_BOUND) ) )
			{print_parm_error(year_text); return FALSE;}
		else
		{
			if (*parm_val <= GDAY_UPPER_BOUND) return TRUE;
			*parm_id = MUST_BE_YEAR;
			*p->ret_year = *parm_val;
		}
	}
	else
	{
		*parm_val = hdate_parse_month_text_string(parm_str);
		if (!*parm_val) {print_parm_error(month_text); return FALSE;}
		*parm_id = MUST_BE_MONTH;
		*p->ret_month = *parm_val;
	}
	return TRUE;
}


/****************************************************
* set_gh_year_month
***************************************************/
void set_gh_year_month( int year, int month, parse_date_struct* p )
{
	// what if month > 12 ? Is that a situation that reaches here?
	if (month > 100)
	{
		*p->ret_year = year + p->base_year_h;
		if ( (month > 112) &&
			 (hdate_get_size_of_hebrew_year (*p->ret_year) < 383 ) )
			*p->ret_month = 106;
	}	
	else *p->ret_year = year + p->base_year_g;
}


/****************************************************
* set_hmonth
***************************************************/
void set_hmonth( int month, parse_date_struct* p )
{
	if (month > GMONTH_UPPER_BOUND)
	{
		if ( (hdate_get_size_of_hebrew_year (*p->ret_year) > 355 ) )
			 *p->ret_month = month + 100;
		else *p->ret_month = 106;
	}
	else *p->ret_month = month + 100;
}



/****************************************************
* second_parse
*   populates *ret_month and *ret_day when we were
*   given three parameters and only could determine
*   that one was MUST_BE_YEAR (ie 32-99 or > 1000)
***************************************************/
int second_parse( int a, int b, parse_date_struct* p)
{
	/// The prejudice here is mm dd, but you can overrule that
	/// by just passing the parameters in reverse.
	if (*p->ret_year >= HEB_YR_LOWER_BOUND)
	{
		if ((a > HDAY_UPPER_BOUND) || (b > HDAY_UPPER_BOUND))
						{print_parm_error(day_text); return FALSE;};
		if (b > HMONTH_UPPER_BOUND)
		{
			if (a > HMONTH_UPPER_BOUND)	{print_parm_error(month_text); return FALSE;}
			*p->ret_month = a + 100;
			*p->ret_day = b;
		}
		else
		{
			*p->ret_month = b + 100;
			*p->ret_day = a;
		}
	}
	else if (*p->ret_year >= GREG_YR_LOWER_BOUND)
	{
		// This should be impossible at this point, because we only
		// get here if both a and b are <= GDAY_UPPER_BOUND
		// if ((a > GDAY_UPPER_BOUND) || (b > GDAY_UPPER_BOUND))
		//				{print_parm_error(day_text); return FALSE;};
		if (a > GMONTH_UPPER_BOUND)
		{
			if (b > GMONTH_UPPER_BOUND)	{print_parm_error(month_text); return FALSE;}
			*p->ret_month = b;
			*p->ret_day = a;
		}
		else
		{
			*p->ret_month = a;
			*p->ret_day = b;
		}
	}
	else /// two digit year 32-99
	{
		if (a > HMONTH_UPPER_BOUND)
		{
			if (b > HMONTH_UPPER_BOUND)	{print_parm_error(month_text); return FALSE;}
			if ( (b > GMONTH_UPPER_BOUND) || (p->prefer_hebrew) )
			{
				*p->ret_month = b + 100;
				*p->ret_day = a;
				*p->ret_year = *p->ret_year + p->base_year_h;
			}
			else
			{
				*p->ret_month = b;
				*p->ret_day = a;
				*p->ret_year = *p->ret_year + p->base_year_g;
			}
		}
		else if (b > HMONTH_UPPER_BOUND)
		{
			if ( (a > GMONTH_UPPER_BOUND) || (p->prefer_hebrew) )
			{
				*p->ret_month = a + 100;
				*p->ret_day = b;
				*p->ret_year = *p->ret_year + p->base_year_h;
			}
			else
			{
				*p->ret_month = a;
				*p->ret_day = b;
				*p->ret_year = *p->ret_year + p->base_year_g;
			}
		}
	}
return TRUE;
}

/****************************************************
* check_two_of_three_gt
***************************************************/
int check_two_of_three_gt( int a, int b, int c, int compare_val)
{
	if ( ( (a > compare_val) && (b > compare_val) ) ||
		 ( (b > compare_val) && (c > compare_val) ) ||
		 ( (c > compare_val) && (a > compare_val) ) )
		 return TRUE;
	return FALSE;
}

/****************************************************
* hebrew_three_parm_parse
***************************************************/
int hebrew_three_parm_parse( parse_date_struct* p )
{	
	/// my personal prejudice to prefer hebrew form dd mm yy
	if (p->b_val > HMONTH_UPPER_BOUND)
	{
		if (p->a_val > HMONTH_UPPER_BOUND)
	 	{
			/// c_val  must be hebrew month
			/// the unattractive choices are yy dd mm or dd yy mm
			if (p->b_val > HDAY_UPPER_BOUND)
			{
				*p->ret_day = p->a_val;
				*p->ret_year = p->b_val + p->base_year_h;
			}
			else
			{
				*p->ret_year = p->a_val + p->base_year_h;
				*p->ret_day = p->b_val;
			}
			set_hmonth( p->c_val, p );
			return TRUE;
		}
		if (p->b_val > HDAY_UPPER_BOUND)
		{
			/// b_val must be year
			*p->ret_year = p->b_val + p->base_year_h;
			if (p->c_val > HMONTH_UPPER_BOUND)
			{
				/// c_val MUST be day
				*p->ret_day = p->c_val;
				set_hmonth( p->a_val, p );
			}
			else /// cover two cases
				 /// 1] if (a_val > HMONTH_UPPER_BOUND)
				 /// 2] both a_val, b_val could be months
			{
				*p->ret_day = p->a_val;
				set_hmonth( p->c_val, p );
			}
			return TRUE;
		}
		*p->ret_day = p->b_val;
		*p->ret_year = p->c_val + p->base_year_h;
		set_hmonth( p->a_val, p );
		return TRUE;
	}
	/// at this point, we know that b_val could be anything
	if (p->a_val > HMONTH_UPPER_BOUND)
	{
		if (p->c_val > HMONTH_UPPER_BOUND)
	 	{
			/// b_val  must be hebrew month
			if (p->a_val > HDAY_UPPER_BOUND)
			{
				*p->ret_year = p->a_val + p->base_year_h;
				*p->ret_day = p->c_val;
			}
			else
			{
				*p->ret_year = p->c_val + p->base_year_h;
				*p->ret_day = p->a_val;
			}
			set_hmonth( p->b_val, p );
			return TRUE;
		}
		if (p->a_val > HDAY_UPPER_BOUND)
		{
			*p->ret_year = p->a_val + p->base_year_h;
			///if (c_val > HMONTH_UPPER_BOUND)
			/// {
			///	*ret_day = c_val;
			/// *ret_month = b_val + 100;
			///	return TRUE;
			///}
			*p->ret_day = p->c_val;
			set_hmonth( p->b_val, p );
			return TRUE;
		}		
	}
	*p->ret_year = p->c_val + p->base_year_h;
	*p->ret_day = p->a_val;
	set_hmonth( p->b_val, p );
	return TRUE;
}

/****************************************************
* gregorian_three_parm_parse
***************************************************/
int gregorian_three_parm_parse()
{
	printf("ERROR: reached function gregorian_three_parm_parse in local_functions, but it hasn't been coded yet\n");
	return TRUE;
}







/************************************************************
* parse_date
* 
* be flexible:
*	Allow yyyy mm OR mm yyyy - check strlen
*	Allow yyyy MMM OR MMM yyyy even with a 2-digit year
* 
* currently returns TRUE always, but if it encounters an
* obvious error, it reports the error and calls exit()
* 
* The logic here is fuzzy and imprecise, and does not cover
* all cases (at least yet). For example, day parameters are
* always evaluated for the gregorian 31-day possibility, and
* the month parameters are always evaluated for the Hebrew
* 14-month possiblity (yes, 13 months, but libhdate internally
* treats ADAR I/II as months 13/14).
* 
	* The base_year variables are for parsing a two-digit
	* year value. Because this is a Hebrew library with
	* Hebrew utilities, we default to a Hebrew year 
	* (prefer_hebrew = TRUE) and presume that the user
	* input and desire is for a Hebrew context.
*
* SEE ALSO: validate_hdate for bounds checking of these values
************************************************************/
	// TODO - it could be that some judgement calls I make
	// should be decided by LC_TIME (set setlocale())
	// #include <locale.h>
	// char* locale_time_settings_string = setlocale(LC_TIME, "");
	// or peek in /usr/share/i18n/locales
	// or something simpler?
int hdate_parse_date( const char* parm_a, const char* parm_b, const char* parm_c,
					 int* ret_year, int* ret_month, int* ret_day, const int parm_cnt,
					 const int prefer_hebrew, const int prefer_2_parm_ym,
					 const int base_year_h, const int base_year_g )
{
	hdate_struct h;
	parse_date_struct p;

	p.a_id = UNKNOWN_STATE;
	p.b_id = UNKNOWN_STATE;
	p.c_id = UNKNOWN_STATE;
	p.a_val = BAD_DATE_VALUE;
	p.b_val = BAD_DATE_VALUE;
	p.c_val = BAD_DATE_VALUE;
	p.ret_year = ret_year;
	p.ret_month = ret_month;
	p.ret_day = ret_day;
	p.prefer_hebrew = prefer_hebrew;
	p.prefer_2_parm_ym = prefer_2_parm_ym;
	p.base_year_g = base_year_g;
	p.base_year_h = base_year_h;
	
	switch (parm_cnt)
	{
	case 3:	if (!initial_parse(parm_c, &p.c_val, &p.c_id, &p )) return FALSE;
			/// and fall through ...
	case 2:	if (!initial_parse(parm_b, &p.b_val, &p.b_id, &p )) return FALSE;
			/// and fall through ...
	case 1:	if (!initial_parse(parm_a, &p.a_val, &p.a_id, &p )) return FALSE;
			break;
	default: return FALSE;
	}

	/// The definite ids from the initial parse are alphabetic months
	/// and four-digit years, and there can only be one of each
	if ( (p.a_id + p.b_id + p.c_id) >= (MUST_BE_MONTH*2) )
	{
		error(0,0,"%s: %s",error_text, N_("multiple month parameters detected"));
		return FALSE;
	}
	if ( ((p.a_id + p.b_id + p.c_id)%10) > MUST_BE_YEAR  )
	{
		error(0,0,"%s: %s",error_text, N_("multiple year parameters detected"));
		return FALSE;
	}

	if (parm_cnt == 1)
	{
		*p.ret_day = 0;
		if (p.a_id == MUST_BE_MONTH)     /// month name was parsed 
		{
			hdate_set_gdate (&h, 0, 0, 0);	/// set date for today
			if (p.a_val > 100)
			{
				*p.ret_year = h.hd_year;
				if ( (p.a_val > 112) && (h.hd_size_of_year < 383 ) )
					*p.ret_month = 106;
			}
			else	*p.ret_year = h.gd_year;
		}
		else if (p.a_id != MUST_BE_YEAR) /// two-digit value < 32
		{
			if (!p.prefer_hebrew)
			{
				if (p.a_val <= GMONTH_UPPER_BOUND)
				{
					*p.ret_month = p.a_val;
					hdate_set_gdate (&h, 0, 0, 0);	/// set date for today
					*p.ret_year = h.gd_year;
				}
				else
				{
					*p.ret_month = 0;
					*p.ret_year = p.a_val + p.base_year_g;
				}
			}
			else /// (p.prefer_hebrew)
			{
				if (p.a_val <= HMONTH_UPPER_BOUND)
				{
					hdate_set_gdate (&h, 0, 0, 0);	/// set date for today
					if ( (p.a_val > GMONTH_UPPER_BOUND) && (h.hd_size_of_year < 383 ) )
					{
						*p.ret_month = 0;
						*p.ret_year = p.a_val + p.base_year_h;
					}
					*p.ret_month = p.a_val + 100;
					*p.ret_year = h.hd_year;;
				}
			}
		}
		return TRUE;
	}

	if (parm_cnt == 2)
	{
		switch (p.a_id + p.b_id)
		{
	default: error(0,0,"%s: %s",error_text, N_("unexpected error in parse_date\n"));
			return FALSE;
			break;
	case 11:	/// MUST_BE_YEAR && MUST_BE_MONTH
			if ( ( (*p.ret_year >= HEB_YR_LOWER_BOUND) && (*p.ret_month < 101) ) ||
			     ( (*p.ret_year <= GREG_YR_UPPER_BOUND) && (*p.ret_month > 100) ) )
			{
				print_parm_mismatch_error();
				return FALSE;
			}
			*p.ret_day = 0;
			if (*p.ret_year <= YR_UPPER_2_BOUND) /// 32-99
				set_gh_year_month( *p.ret_year, *p.ret_month, &p );
			// TODO - test that validate_date faults day 31 and some day 30 in Hebrew month
			break;
	case 10:	/// MUST_BE_MONTH
			if (p.a_id == MUST_BE_MONTH) *p.ret_year = p.b_val;
			else  *p.ret_year = p.a_val;
			if (p.prefer_2_parm_ym)
			{
				*p.ret_day = 0;
				set_gh_year_month( *p.ret_year, *p.ret_month, &p );
			}
			else /// prefer two parm as dd mm
			{
				/// *p.ret_year is temporarily
				/// holding the expected day value
				if (*p.ret_month > 100)
				{
					if (*p.ret_year > HDAY_UPPER_BOUND)
					{
						*p.ret_day = 0;
						set_gh_year_month( *p.ret_year, *p.ret_month, &p );
					}
					else
					{
						*p.ret_day = *p.ret_year;
						hdate_set_gdate (&h, 0, 0, 0);	/// set date for today
						*p.ret_year = h.hd_year;
					}
				}
				else
				{
					*p.ret_day = *p.ret_year;
					hdate_set_gdate (&h, 0, 0, 0);	/// set date for today
					*p.ret_year = h.gd_year;
				}	
			}
			break;
	case  1:	/// MUST_BE_YEAR
			/// since we have only two parms, and one must be a year
			/// we insist that the other be a month
			*p.ret_day = 0;
			if (p.a_id == MUST_BE_YEAR)
				 *p.ret_month = p.b_val;
			else *p.ret_month = p.a_val;
			if (*p.ret_month > HMONTH_UPPER_BOUND)
			{
				print_parm_error(month_text);
				return FALSE;
			}
			if (*p.ret_year >= HEB_YR_LOWER_BOUND)
				set_hmonth( *p.ret_month, &p );
			else if ( (*p.ret_year >= YR_LOWER_4_BOUND) &&
					  (*p.ret_month > GMONTH_UPPER_BOUND) )
			{
				print_parm_error(month_text);
				return FALSE;
			}
			else if (*p.ret_year <= YR_UPPER_2_BOUND) 
			{
				if ( (p.prefer_hebrew) || 
					 (*p.ret_month > GMONTH_UPPER_BOUND) )
				{
					*p.ret_year = *p.ret_year + p.base_year_h;
					set_hmonth( *p.ret_month, &p );
				}
				else *p.ret_year = *p.ret_year + p.base_year_g;
			}
			return TRUE;
			break;
	case  0: /// both parms are in UNKNOWN_STATE
			 /// ie. numeric, 0 < n < 32
			if ((p.a_val > HMONTH_UPPER_BOUND) && (p.b_val > HMONTH_UPPER_BOUND)) 
				{print_parm_error(month_text); return FALSE;}
			if ((p.prefer_hebrew) ||
				((p.a_val > GMONTH_UPPER_BOUND) && (p.b_val > GMONTH_UPPER_BOUND)))
			{
				*p.ret_year = p.a_val + base_year_h;
				if (p.a_val > HMONTH_UPPER_BOUND)
					*p.ret_month = p.b_val;
				else *p.ret_month = p.a_val; /// preference of mm yy
				set_hmonth( *p.ret_month, &p );
			}
			else /// prefer gregorian
			{
				*p.ret_year = p.a_val + p.base_year_g;
				if (p.b_val > GMONTH_UPPER_BOUND)
					 *p.ret_month = p.a_val;
				else *p.ret_month = p.b_val;
			}
		}
		return TRUE;
	}

	/// three parameters
	switch (p.a_id + p.b_id + p.c_id)
	{
	default: error(0,0,"%s: %s",error_text, N_("unexpected error in parse_date\n"));
			return FALSE;
			break;
	case 11:	/// MUST_BE_YEAR && MUST_BE_MONTH
			if ( ( (*p.ret_year >= HEB_YR_LOWER_BOUND) && (*p.ret_month < 101) ) ||
			     ( (*p.ret_year <= GREG_YR_UPPER_BOUND) && (*p.ret_month > 100) ) )
			{
				print_parm_mismatch_error();
				return FALSE;
			}
			if (*p.ret_year <= YR_UPPER_2_BOUND) /// 32-99
				set_gh_year_month( *p.ret_year, *p.ret_month, &p );
			if (p.a_id == UNKNOWN_STATE) *p.ret_day = p.a_val;
			else if (p.b_id == UNKNOWN_STATE) *p.ret_day = p.b_val;
			else *p.ret_day = p.c_val;
			// TODO - test that validate_date faults day 31 and some day 30 in Hebrew month
			break;
	case 10:	/// MUST_BE_MONTH
			if (p.a_id == MUST_BE_MONTH)
			{
				/// personal prejudice to prefer mmmm dd yy
				*p.ret_day = p.b_val; // TODO - test that validate_date faults day 31 and some day 30 in Hebrew month
				set_gh_year_month( p.c_val, p.a_val, &p );
			}
			else if (p.b_id == MUST_BE_MONTH)
			{
				/// prefer dd mmmm yy because its more natural in Hebrew though
				/// had it been three numeric fields, I would favor yy mm dd
				*p.ret_day = p.a_val; // TODO - test that validate_date faults day 31 and some day 30 in Hebrew month
				set_gh_year_month( p.c_val, p.b_val, &p );
			}
			else ///(p.c_id == MUST_BE_MONTH)
			{
				/// tough call - my intuition is yy dd mmmm
				*p.ret_day = p.b_val; // TODO - test that validate_date faults day 31 and some day 30 in Hebrew month
				set_gh_year_month( p.a_val, p.c_val, &p );
			}
			break;
	case  1:	/// MUST_BE_YEAR
			if      (p.a_id == MUST_BE_YEAR) return second_parse(p.b_val, p.c_val, &p );
			else if (p.b_id == MUST_BE_YEAR) return second_parse(p.a_val, p.c_val, &p );
			else return second_parse(p.a_val, p.b_val, &p );
			break;
	case  0: /// all three parms are in UNKNOWN_STATE
			 /// ie. numeric, 0 < n < 32
			if ((p.a_val > HMONTH_UPPER_BOUND) &&
				(p.b_val > HMONTH_UPPER_BOUND) &&
				(p.c_val > HMONTH_UPPER_BOUND) )
			{
				print_parm_error(month_text);
				return FALSE;
			}
			if ( (p.a_val > GMONTH_UPPER_BOUND) &&
				 (p.b_val > GMONTH_UPPER_BOUND) &&
				 (p.c_val > GMONTH_UPPER_BOUND) )
			{
				/// MUST be hebrew
				if ( check_two_of_three_gt( p.a_val, p.b_val, p.c_val, HDAY_UPPER_BOUND ) )
				{
					print_parm_mismatch_error();
					return FALSE;
				}
				return hebrew_three_parm_parse( &p );
			}
			if ( check_two_of_three_gt( p.a_val, p.b_val, p.c_val, HDAY_UPPER_BOUND ) )
			{
				return gregorian_three_parm_parse();
			}
			if ( (p.prefer_hebrew) &&
				 ( ! check_two_of_three_gt( p.a_val, p.b_val, p.c_val, HDAY_UPPER_BOUND ) ) )
				return hebrew_three_parm_parse( &p );
			else return gregorian_three_parm_parse();
			break;
	} /// end switch (p.a_id + p.b_id + p.c_id)
	return TRUE;
}
