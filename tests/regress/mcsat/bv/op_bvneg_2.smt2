(set-logic QF_BV)
(set-info :status sat)

(declare-fun x () (_ BitVec 10))
(declare-fun y () (_ BitVec 10))
(declare-fun z () (_ BitVec 10))

(declare-fun o1 () (_ BitVec 10))
(declare-fun o2 () (_ BitVec 10))
(declare-fun o3 () (_ BitVec 10))

(assert (= x #b0000000000))
(assert (= y #b1111111111))
(assert (= z #b0101001010))

(assert (= o1 (bvneg x)))
(assert (= o2 (bvneg y)))
(assert (= o3 (bvneg z)))

(check-sat)

(exit)

