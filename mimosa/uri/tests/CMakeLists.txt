mimosa_test(mimosa-uri
  tst-normalize-path.cc
  tst-parse-query.cc
  tst-percent-encoding.cc
  tst-url.cc)
target_link_libraries(tst-mimosa-uri mimosa-uri)
