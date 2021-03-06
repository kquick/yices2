(set-logic QF_BV)
(declare-fun x () (_ BitVec 3))
(declare-fun y () (_ BitVec 8))
(declare-fun z () (_ BitVec 8))
(declare-fun w () (_ BitVec 8))
(assert 
  (let ((e5 (bvsub y w))) 
  (let ((e6 (ite (bvuge ((_ sign_extend 5) x) e5) (_ bv1 1) (_ bv0 1)))) 
  (let ((e7 (bvlshr e5 e5))) 
  (let ((e8 (bvxor y z))) 
  (let ((e10 (bvult y ((_ sign_extend 7) e6)))) 
  (let ((e12 (= y ((_ zero_extend 5) x)))) 
  (let ((e14 (bvule e7 e8))) 
  (let ((e28 (xor false e14))) 
  (let ((e29 (=> e28 false))) 
  (let ((e33 (and e12 e10))) 
  (let ((e39 (and e33 e29))) 
  (let ((e45 (=> true e39))) e45)))))))))))))
(check-sat)

