(set-info :source |fuzzsmt|)
(set-info :smt-lib-version 2.0)
(set-info :category "random")
(set-info :status unknown)
(set-logic QF_IDL)
(declare-fun v0 () Int)
(declare-fun v1 () Int)
(assert (let ((e2 61895))
(let ((e3 329848102180743))
(let ((e4 764349))
(let ((e5 23463))
(let ((e6 2))
(let ((e7 6))
(let ((e8 (< v0 v1)))
(let ((e9 (distinct (- v0 v0) e3)))
(let ((e10 (= (- v1 v1) e7)))
(let ((e11 (= v0 v0)))
(let ((e12 (>= (- v0 v1) e6)))
(let ((e13 (>= (- v0 v1) e7)))
(let ((e14 (<= v0 v0)))
(let ((e15 (> (- v0 v0) (- e2))))
(let ((e16 (>= v0 v0)))
(let ((e17 (<= v0 v0)))
(let ((e18 (= (- v1 v1) (- e2))))
(let ((e19 (distinct (- v1 v0) e3)))
(let ((e20 (> (- v1 v1) e4)))
(let ((e21 (> v1 v1)))
(let ((e22 (<= (- v1 v0) (- e4))))
(let ((e23 (< (- v1 v0) (- e6))))
(let ((e24 (<= (- v0 v1) (- e4))))
(let ((e25 (>= (- v1 v0) (- e4))))
(let ((e26 (< v0 v1)))
(let ((e27 (> (- v0 v1) (- e3))))
(let ((e28 (distinct v1 v1)))
(let ((e29 (distinct (- v0 v0) (- e3))))
(let ((e30 (< v0 v0)))
(let ((e31 (< (- v1 v0) (- e7))))
(let ((e32 (>= v0 v0)))
(let ((e33 (< (- v1 v1) e4)))
(let ((e34 (<= (- v1 v0) (- e4))))
(let ((e35 (= (- v1 v0) (- e5))))
(let ((e36 (> v0 v0)))
(let ((e37 (> v0 v0)))
(let ((e38 (< v1 v1)))
(let ((e39 (>= (- v1 v1) (- e4))))
(let ((e40 (= v0 v1)))
(let ((e41 (distinct v1 v0)))
(let ((e42 (< v0 v0)))
(let ((e43 (< v1 v1)))
(let ((e44 (< (- v0 v1) (- e4))))
(let ((e45 (= (- v0 v0) e2)))
(let ((e46 (<= (- v1 v0) (- e5))))
(let ((e47 (distinct (- v0 v1) e2)))
(let ((e48 (>= (- v0 v0) e4)))
(let ((e49 (< v1 v0)))
(let ((e50 (> v0 v1)))
(let ((e51 (> v0 v1)))
(let ((e52 (distinct (- v1 v1) e5)))
(let ((e53 (distinct (- v1 v1) (- e2))))
(let ((e54 (<= (- v1 v1) (- e4))))
(let ((e55 (< (- v0 v1) e4)))
(let ((e56 (= (- v0 v0) e5)))
(let ((e57 (= v0 v0)))
(let ((e58 (= (- v1 v0) (- e4))))
(let ((e59 (> v1 v1)))
(let ((e60 (distinct v1 v0)))
(let ((e61 (distinct (- v0 v0) (- e7))))
(let ((e62 (ite e25 e46 e56)))
(let ((e63 (and e51 e53)))
(let ((e64 (=> e47 e10)))
(let ((e65 (= e43 e52)))
(let ((e66 (=> e31 e65)))
(let ((e67 (= e24 e13)))
(let ((e68 (= e28 e38)))
(let ((e69 (or e39 e30)))
(let ((e70 (and e64 e55)))
(let ((e71 (=> e11 e29)))
(let ((e72 (or e69 e21)))
(let ((e73 (= e22 e49)))
(let ((e74 (or e33 e18)))
(let ((e75 (not e67)))
(let ((e76 (xor e15 e74)))
(let ((e77 (xor e14 e71)))
(let ((e78 (= e36 e23)))
(let ((e79 (= e34 e58)))
(let ((e80 (and e68 e62)))
(let ((e81 (=> e50 e63)))
(let ((e82 (not e42)))
(let ((e83 (not e77)))
(let ((e84 (ite e45 e16 e60)))
(let ((e85 (= e76 e82)))
(let ((e86 (= e84 e80)))
(let ((e87 (or e85 e12)))
(let ((e88 (xor e83 e70)))
(let ((e89 (ite e66 e17 e48)))
(let ((e90 (ite e79 e27 e86)))
(let ((e91 (and e73 e44)))
(let ((e92 (ite e81 e87 e87)))
(let ((e93 (ite e32 e72 e78)))
(let ((e94 (ite e90 e40 e59)))
(let ((e95 (xor e19 e75)))
(let ((e96 (ite e95 e94 e94)))
(let ((e97 (=> e35 e93)))
(let ((e98 (and e26 e61)))
(let ((e99 (xor e8 e37)))
(let ((e100 (and e20 e92)))
(let ((e101 (= e41 e88)))
(let ((e102 (xor e54 e57)))
(let ((e103 (or e91 e9)))
(let ((e104 (and e89 e101)))
(let ((e105 (=> e100 e103)))
(let ((e106 (not e97)))
(let ((e107 (and e106 e105)))
(let ((e108 (= e107 e102)))
(let ((e109 (=> e98 e98)))
(let ((e110 (xor e104 e104)))
(let ((e111 (ite e99 e108 e109)))
(let ((e112 (and e111 e96)))
(let ((e113 (or e112 e110)))
e113
)))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))

(check-sat)