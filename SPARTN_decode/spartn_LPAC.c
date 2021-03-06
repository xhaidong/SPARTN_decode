#include "spartn.h"
#include "log.h"
#include "bits.h"
#include <string.h>

FILE*  lpac_table_file = NULL;

void log_lpac_title_to_table() {
	table_log_ex(lpac_table_file, "%9s,%5s,%5s,%5s,%5s,%5s,%5s,%5s,%7s,%12s", "Time", "area", "lat", "lon", "lat_c", "lon_c", "lat_s", "lon_s", "VTEC", "residual...");
}

void open_lpac_table_file(const char* filename) {
	if (filename) {
		open_table_file_ex(&lpac_table_file, filename);
	}
	else {
		open_table_file_ex(&lpac_table_file, "../LPAC_message.log");
	}
	log_lpac_title_to_table();
}

void close_lpac_table_file() {
	close_table_file_ex(&lpac_table_file);
}

void log_lpac_to_table(raw_spartn_t* spartn, LPAC_t* lpac) {
	uint32_t i,j;
	uint32_t time = spartn->GNSS_time_type;
	for (i = 0; i < lpac->header.SF071_LPAC_area_count; i++) {
		LPAC_area_t* area = &lpac->areas[i];
		uint32_t mask_len = area->SF075_LPAC_area_latitude_grid_node_count * area->SF076_LPAC_area_longitude_grid_node_count;
		char VTEC_array[1024] = { 0 };
		for (j = 0; j < mask_len; j++) {
			char VTEC[8] = { 0 };
			sprintf(VTEC, "%7.3f", area->VTEC[j].SF082_VTEC_residual);
			if (j != 0) {
				strcat(VTEC_array, ",");
			}
			strcat(VTEC_array, VTEC);
		}
		table_log_ex(lpac_table_file, "%9d,%5d,%5d,%5d,%5d,%5d,%5d,%5d,%7.3f,%s", time, area->SF072_LPAC_area_ID,
			area->SF073_LPAC_area_reference_latitude, area->SF074_LPAC_area_reference_longitude,
			area->SF075_LPAC_area_latitude_grid_node_count, area->SF076_LPAC_area_longitude_grid_node_count,
			area->SF077_LPAC_area_latitude_grid_node_spacing, area->SF078_LPAC_area_longitude_grid_node_spacing,
			area->SF080_Average_area_VTEC, VTEC_array);
	}
	table_log_ex(lpac_table_file, "");
}
//Table 6.25 LPAC grid node VTEC block
void decode_LPAC_grid_node_VTEC_block(raw_spartn_t* spartn, LPAC_VTEC_t* VTEC, int tab) {
	uint8_t* payload = spartn->payload;
	VTEC->SF055_VTEC_quality = getbitu(payload, spartn->offset, 4); spartn->offset += 4; slog(LOG_DEBUG, tab, "SF055_VTEC_quality = %d", VTEC->SF055_VTEC_quality);
	VTEC->SSF081_VTEC_size_indicator = getbitu(payload, spartn->offset, 1); spartn->offset += 1; slog(LOG_DEBUG, tab, "SSF081_VTEC_size_indicator = %d", VTEC->SSF081_VTEC_size_indicator);
	if (VTEC->SSF081_VTEC_size_indicator) {
		//SF083
		VTEC->SF083_VTEC_residual = getbitu(payload, spartn->offset, 11)*0.25 - 255.75; spartn->offset += 11; slog(LOG_DEBUG, tab, "SF083_VTEC_residual = %f", VTEC->SF083_VTEC_residual);
	}
	else {
		//SF082
		VTEC->SF082_VTEC_residual = getbitu(payload, spartn->offset, 7)*0.25 - 15.75; spartn->offset += 7; slog(LOG_DEBUG, tab, "SF082_VTEC_residual = %f", VTEC->SF082_VTEC_residual);
	}
}

//Table 6.23 LPAC area block
void decode_LPAC_area_block(raw_spartn_t* spartn, LPAC_area_t* area,int tab) {
	uint32_t i;
	uint8_t* payload = spartn->payload;
	//Table 6.24 LPAC area data block 
	area->SF072_LPAC_area_ID = getbitu(payload, spartn->offset, 2); spartn->offset += 2; slog(LOG_DEBUG, tab, "SF072_LPAC_area_ID = %d", area->SF072_LPAC_area_ID);
	area->SF073_LPAC_area_reference_latitude = getbitu(payload, spartn->offset, 8) - 85; spartn->offset += 8; slog(LOG_DEBUG, tab, "SF073_LPAC_area_reference_latitude = %d", area->SF073_LPAC_area_reference_latitude);
	area->SF074_LPAC_area_reference_longitude = getbitu(payload, spartn->offset, 9) - 180; spartn->offset += 9; slog(LOG_DEBUG, tab, "SF074_LPAC_area_reference_longitude = %d", area->SF074_LPAC_area_reference_longitude);
	area->SF075_LPAC_area_latitude_grid_node_count = getbitu(payload, spartn->offset, 4) + 1; spartn->offset += 4; slog(LOG_DEBUG, tab, "SF075_LPAC_area_latitude_grid_node_count = %d", area->SF075_LPAC_area_latitude_grid_node_count);
	area->SF076_LPAC_area_longitude_grid_node_count = getbitu(payload, spartn->offset, 4) + 1; spartn->offset += 4; slog(LOG_DEBUG, tab, "SF076_LPAC_area_longitude_grid_node_count = %d", area->SF076_LPAC_area_longitude_grid_node_count);
	area->SF077_LPAC_area_latitude_grid_node_spacing = getbitu(payload, spartn->offset, 2); spartn->offset += 2; slog(LOG_DEBUG, tab, "SF077_LPAC_area_latitude_grid_node_spacing = %d", area->SF077_LPAC_area_latitude_grid_node_spacing);
	area->SF078_LPAC_area_longitude_grid_node_spacing = getbitu(payload, spartn->offset, 2); spartn->offset += 2; slog(LOG_DEBUG, tab, "SF078_LPAC_area_longitude_grid_node_spacing = %d", area->SF078_LPAC_area_longitude_grid_node_spacing);
	area->SF080_Average_area_VTEC = getbitu(payload, spartn->offset, 12)*0.25 - 511.75; spartn->offset += 12; slog(LOG_DEBUG, tab, "SF080_Average_area_VTEC = %f", area->SF080_Average_area_VTEC);
	uint32_t mask_len = area->SF075_LPAC_area_latitude_grid_node_count * area->SF076_LPAC_area_longitude_grid_node_count; slog(LOG_DEBUG, tab, "mask_len = %d", mask_len);
	uint8_t SF079_Grid_node_present_mask[32] = { 0 };
	bitscopy(SF079_Grid_node_present_mask, 0, payload, spartn->offset, mask_len); spartn->offset += mask_len;
	bits_to_bytes_array(SF079_Grid_node_present_mask, area->SF079_Grid_node_present_mask, mask_len);
	slog(LOG_DEBUG, tab, "");
	//Table 6.25 LPAC grid node VTEC block  (Repeated)
	for (i = 0; i < mask_len; i++) {
		if (area->SF079_Grid_node_present_mask[i]) {
			LPAC_VTEC_t* VTEC = &(area->VTEC[i]);
			decode_LPAC_grid_node_VTEC_block(spartn, VTEC, tab+1);
		}
	}
}
void decode_LPAC_header_block(raw_spartn_t* spartn, LPAC_header_t* header,int tab) {
	uint8_t* payload = spartn->payload;
	header->SF005_SIOU = getbitu(payload, spartn->offset, 9); spartn->offset += 9; slog(LOG_DEBUG, tab, "SF005_SIOU = %d", header->SF005_SIOU);
	header->SF069_Reserved = getbitu(payload, spartn->offset, 1);  spartn->offset += 1; slog(LOG_DEBUG, tab, "SF069_Reserved = %d", header->SF069_Reserved);
	header->SF070_Ionosphere_shell_height = getbitu(payload, spartn->offset, 2);  spartn->offset += 2; slog(LOG_DEBUG, tab, "SF070_Ionosphere_shell_height = %d", header->SF070_Ionosphere_shell_height);
	header->SF071_LPAC_area_count = getbitu(payload, spartn->offset, 2) + 1;  spartn->offset += 2; slog(LOG_DEBUG, tab, "SF071_LPAC_area_count = %d", header->SF071_LPAC_area_count);
}
// SM 3-0 LPAC messages 
extern int decode_LPAC_message(raw_spartn_t* spartn)
{
	if (!spartn) return 0;
	if (!spartn->spartn_out) return 0;
	if (!spartn->spartn_out->lpac) return 0;
	int i, tab = 2;
	spartn->payload = spartn->buff + spartn->Payload_offset;
	spartn->offset = 0;
	LPAC_t* lpac = spartn->spartn_out->lpac;
	memset(lpac, 0, sizeof(LPAC_t));
	//Table 6.22 Header block
	LPAC_header_t* header = &(lpac->header);
	decode_LPAC_header_block(spartn, header, tab);
	//Table 6.23 LPAC area block (Repeated)
	for (i = 0; i < header->SF071_LPAC_area_count; i++) {
		LPAC_area_t* area = &(lpac->areas[i]);
		decode_LPAC_area_block(spartn, area, tab+1);
	}
	transform_spartn_ssr(spartn);
	slog(LOG_DEBUG, tab, "offset = %d bits", spartn->offset);
	slog(LOG_DEBUG, tab, "size of LPAC_t = %d ", sizeof(LPAC_t));
	log_lpac_to_table(spartn, lpac);
	return 1;
}