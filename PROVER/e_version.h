/*-----------------------------------------------------------------------

File  : e_version.h

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Define global macros for version number and meta-information.

  Copyright 2009-2024 by the authors (see DOC/CONTRIBUTORS).
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

-----------------------------------------------------------------------*/

#ifndef E_VERSION

#define E_VERSION


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

/* git tag E-3.0.1 */

#define PVERSION      "3.0.1"

#include "e_gitcommit.h"

#ifdef ENABLE_LFHO
#define LFH "-ho"
#else
#define LFH ""
#endif

#ifdef NDEBUG
#define VERSION PVERSION LFH
#else
#define VERSION PVERSION LFH "-DEBUG"
#endif

#define E_NICKNAME     "Countess Grey"

#define E_URL "http://www.eprover.org"

#define STS_MAIL "schulz@eprover.org"
#define STS_COPYRIGHT "Copyright 1998-2024 by Stephan Schulz"
#define CTR_COPYRIGHT "and the E contributors (see DOC/CONTRIBUTORS)."
#define STS_SNAIL "Stephan Schulz\nDHBW Stuttgart\nFakultaet Technik\nInformatik\nLerchenstrasse 1\n70174 Stuttgart\nGermany\n"
#define HO_MAIL "jasmin.blanchette@gmail.com"

#define E_FOOTER \
STS_COPYRIGHT", " STS_MAIL ",\n\
"CTR_COPYRIGHT"\n\
\n\
This program is a part of the distribution of the equational theorem\n\
prover E. You can find the latest version of the E distribution\n\
as well as additional information at\n"\
E_URL\
"\n\n"\
"This program is free software; you can redistribute it and/or modify\n\
it under the terms of the GNU General Public License as published by\n\
the Free Software Foundation; either version 2 of the License, or\n\
(at your option) any later version.\n\
\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License for more details.\n\
\n\
You should have received a copy of the GNU General Public License\n\
along with this program (it should be contained in the top level\n\
directory of the distribution in the file COPYING); if not, write to\n\
the Free Software Foundation, Inc., 59 Temple Place, Suite 330,\n\
Boston, MA  02111-1307 USA\n\
\n\
We welcome bug reports and even reasonable questions. If the prover\n\
behaves in an unexpected way, please include the following\n\
information:\n\
\n\
- What did you observe?\n\
- What did you expect?\n\
- The output of `eprover --version`\n\
- The full commandline that lead to the unexpected behaviour\n\
- The input file(s) that lead to the unexpected behaviour\n\
\n\
Most bug reports should be send to <"STS_MAIL">. Bug reports with \n\
respect to the HO-version should be send to or at least copied to \n\
<"HO_MAIL">. Please remember that this is an unpaid\n\
volunteer service.\n\
\n\
The original copyright holder can be contacted via email or as\n\
\n" STS_SNAIL "\n"


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
