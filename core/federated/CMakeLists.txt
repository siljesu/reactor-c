set(FEDERATED_SOURCES clock-sync.c federate.c net_util.c RTI/rti_lib.c RTI/message_record/message_record.c)
list(APPEND INFO_SOURCES ${FEDERATED_SOURCES})

list(TRANSFORM FEDERATED_SOURCES PREPEND federated/)
target_sources(core PRIVATE ${FEDERATED_SOURCES})
