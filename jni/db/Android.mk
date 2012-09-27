LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := db
LOCAL_SRC_FILES := \
	sys_time.cpp \
	db_eco_table.cpp \
	db_annotation.cpp \
	db_board.cpp \
	db_board_base.cpp \
	db_clock.cpp \
	db_comment.cpp \
	db_common.cpp \
	db_consumer.cpp \
	db_database.cpp \
	db_database_content.cpp \
	db_database_codec.cpp \
	db_document_writer.cpp \
	db_date.cpp \
	db_eco.cpp \
	db_edit_key.cpp \
	db_edit_node.cpp \
	db_engine_list.cpp \
	db_exception.cpp \
	db_filter.cpp \
	db_game.cpp \
	db_game_data.cpp \
	db_game_info.cpp \
	db_guess.cpp \
	db_guess_eval.cpp \
	db_home_pawns.cpp \
	db_info_consumer.cpp \
	db_line.cpp \
	db_log.cpp \
	db_mark.cpp \
	db_mark_set.cpp \
	db_move.cpp \
	db_move_info.cpp \
	db_move_info_set.cpp \
	db_move_list.cpp \
	db_move_node.cpp \
	db_namebase.cpp \
	db_namebase_entry.cpp \
	db_namebases.cpp \
	db_pgn_reader.cpp \
	db_pgn_writer.cpp \
	db_player.cpp \
	db_player_stats.cpp \
	db_probe.cpp \
	db_producer.cpp \
	db_provider.cpp \
	db_query.cpp \
	db_search.cpp \
	db_selector.cpp \
	db_signature.cpp \
	db_site.cpp \
	db_statistic.cpp \
	db_tablebase.cpp \
	db_tag_set.cpp \
	db_tournament_table.cpp \
	db_tree.cpp \
	db_tree_info.cpp \
	db_tree_cache.cpp \
	db_time.cpp \
	db_var_consumer.cpp \
	db_writer.cpp \
	cbh/cbh_codec.cpp \
	cbh/cbh_decoder.cpp \
	cbh/cbh_decoder_position.cpp \
	sci/sci_codec.cpp \
	sci/sci_consumer.cpp \
	sci/sci_decoder.cpp \
	sci/sci_decoder_position.cpp \
	sci/sci_encoder.cpp \
	sci/sci_encoder_position.cpp \


LOCAL_CFLAGS    := \
	-mandroid \
	-DTARGET_OS=android -D__ANDROID__ \
	-isystem $(SYSROOT)/usr/include
	-DNO_PREFETCH=1 \
	-DSCI_NAMEBASE_FIX=1

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../mstl $(LOCAL_PATH)/../util $(LOCAL_PATH)/../universalchardet $(LOCAL_PATH)/sci $(LOCAL_PATH)/cbh

LOCAL_STATIC_LIBRARIES := util mstl universalchardet stlport

include $(BUILD_SHARED_LIBRARY)

