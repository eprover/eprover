
(prover
  (name read-tptp-status)
  (cmd "grep '%[ ]*Status[ ]*:[ ]*' $file")
  (sat "Satisfiable|CounterSatisfiable")
  (unsat "Unsatisfiable|Theorem")
  (unknown "Unknown"))

(prover
  (name eprover-dev)
  (binary $cur_dir/PROVER/eprover)
  (cmd "$cur_dir/PROVER/eprover --auto $file --cpu-limit=$timeout")
  (sat "SZS status[ ]*(Satisfiable|CounterSatisfiable)[ ]*$")
  (unsat "SZS status[ ]*(Unsatisfiable|Theorem)[ ]*$"))

(prover
  (name eprover-ho-dev)
  (binary $cur_dir/PROVER/eprover-ho)
  (cmd "$cur_dir/PROVER/eprover-ho --auto $file --cpu-limit=$timeout")
  (sat "SZS status[ ]*(Satisfiable|CounterSatisfiable)[ ]*$")
  (unsat "SZS status[ ]*(Unsatisfiable|Theorem)[ ]*$"))

(dir
  (path $cur_dir/EXAMPLE_PROBLEMS/)
  (pattern ".*\\.p$")
  (expect (try (run read-tptp-status) (const unknown))))

(task
  (name eprover-quick-test)
  (synopsis "quick test of local E")
  (action
   (run_provers
    (provers eprover-dev eprover-ho-dev)
    (dirs $cur_dir/EXAMPLE_PROBLEMS)
    (timeout 10))))
