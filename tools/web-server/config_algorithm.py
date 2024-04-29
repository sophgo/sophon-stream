import samples.license_area_intrusion.license_area_intrusion as license_area_intrusion
import samples.tripwire.tripwire as tripwire

map_type={1:'license_area_intrusion', 2:'tripwire'}
class Algorithms:
    def tripwire_build_config(self,algorithm_name,stream_path,data,port,i):
        return tripwire.tripwire_build_config(algorithm_name,stream_path,data,port,i)
    def tripwire_trans_json(self,json_data,task_id,Type,up_list):
        return tripwire.tripwire_trans_json(json_data,task_id,Type,up_list)
    def tripwire_logic(self,json_data,up_list,rm_list):
        return tripwire.tripwire_logic(json_data,up_list,rm_list)
    def license_area_intrusion_build_config(self,algorithm_name,stream_path,data,port,i):
        return license_area_intrusion.license_area_intrusion_build_config(algorithm_name,stream_path,data,port,i)
    def license_area_intrusion_trans_json(self,json_data,task_id,Type,up_list):
        return license_area_intrusion.license_area_intrusion_trans_json(json_data,task_id,Type,up_list)
    def license_area_intrusion_logic(self,json_data,up_list,rm_list):
        return license_area_intrusion.license_area_intrusion_logic(json_data,up_list,rm_list)
    