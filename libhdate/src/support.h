/** support.h            http://libhdate.sourceforge.net
 *  headers and macros for hcal and hdate (part of package libhdate)
 *
 *  Copyright (c): 2012  Baruch Baum
 *                 2004-2007  Yaacov Zamir <kzamir@walla.co.il>
 *                 1984-2003 Amos Shapir
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

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE -1
#endif


/// Standard gettext macros.
#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif

/// support for fnmatch
#define FNM_EXTMATCH	(1 << 5)
#define FNM_NOFLAG		0

#define _GNU_SOURCE				/// For mempcpy, asprintf
								//  This overrides a setting in the
								//  make file - maybe change it there...

#define _POSIX_C_SOURCE 200809L /// for stat


#define BAD_DATE_VALUE -1
#define BAD_COORDINATE	999
#define BAD_TIMEZONE	999
#define DELTA_LONGITUDE 30

#define HEB_YR_UPPER_BOUND 10999
#define HEB_YR_LOWER_BOUND 3000
#define JUL_DY_LOWER_BOUND 348021
