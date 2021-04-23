cmake_minimum_required(VERSION 3.5)

set(EOSIO_VERSION_MIN "3.1")
set(EOSIO_VERSION_SOFT_MAX "3.1")
# set(EOSIO_VERSION_HARD_MAX "")

find_package(leap)

# Check the version of Leap
set(VERSION_MATCH_ERROR_MSG "")
eosio_check_version(VERSION_OUTPUT "${EOSIO_VERSION}" "${EOSIO_VERSION_MIN}" "${EOSIO_VERSION_SOFT_MAX}"
                    "${EOSIO_VERSION_HARD_MAX}" VERSION_MATCH_ERROR_MSG)
if(VERSION_OUTPUT STREQUAL "MATCH")
  message(STATUS "Using Leap version ${EOSIO_VERSION}")
elseif(VERSION_OUTPUT STREQUAL "WARN")
  message(
    WARNING
      "Using Leap version ${EOSIO_VERSION} even though it exceeds the maximum supported version of ${EOSIO_VERSION_SOFT_MAX}; continuing with configuration, however build may fail.\nIt is recommended to use Leap version ${EOSIO_VERSION_SOFT_MAX}.x"
  )
else() # INVALID OR MISMATCH
  message(
    FATAL_ERROR
      "Found Leap version ${EO