define make_subdir
 @for d in $1; do \
  (make -C $$d $2) \
 done;
endef

define run_test
 @for a in $^; do \
  echo "====== Running test: $$a ======"; \
  ./$$a && echo "====== SUCCESS ======" || echo "====== FAILURE ======"; \
 done
endef
