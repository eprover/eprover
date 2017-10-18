/*-----------------------------------------------------------------------

File  : pcl_proofcheck.h

Author: Stephan Schulz

Contents

  Data types and algorithms to realize proof checking for PCL2
  protocols.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Apr  3 22:49:51 GMT 2000
    New

-----------------------------------------------------------------------*/

#ifndef PCL_PROOFCHECK

#define PCL_PROOFCHECK

#include <cio_tempfile.h>
#include <pcl_protocol.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef enum
{
   CheckFail,
   CheckOk,
   CheckByAssumption,
   CheckNotImplemented
}PCLCheckType;

typedef enum
{
   NoProver,
   EProver,
   Spass,
   Setheo,
   Otter
}ProverType;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define E_EXEC_DEFAULT     "eprover"
#define OTTER_EXEC_DEFAULT "otter"
#define SPASS_EXEC_DEFAULT "SPASS-0.55"

long PCLCollectPreconds(PCLProt_p prot, PCLStep_p step, ClauseSet_p set);
long PCLNegSkolemizeClause(PCLProt_p prot, PCLStep_p step,
            ClauseSet_p set);
ClauseSet_p PCLGenerateCheck(PCLProt_p prot, PCLStep_p step);

PCLCheckType PCLStepCheck(PCLProt_p prot, PCLStep_p step, ProverType
           prover, char* executable, long time_limit);

long PCLProtCheck(PCLProt_p prot, ProverType
        prover, char* executable, long time_limit, long*
        unchecked);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





