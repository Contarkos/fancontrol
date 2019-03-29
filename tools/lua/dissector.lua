do
  -- Port d'ecoute pour dissecter les messages
  local port = {
    21001,
    31001,
  }

  -- Dissector permettant de lire les messages envoyés par REMOTE
  local fan = Proto("fan", "FAN CONTROL")
  local f = fan_control.fields

  -----------------------------------------------------------
  -- Definition des champs pour les messages
  
  -- MAIN
  f.main_shutdown = ProtoField.bytes("fan.main_shutdown", "MAIN_SHUTDOWN")
  
  -- FAN
  f.fan_mode = ProtoField.bytes("fan.fan_mode", "FAN_MODE")
  f.fan_mode_mode = ProtoField.int8("fan.fan_mode_mode", "mode", base.DEC, {
      [0] = "FAN_MODE_AUTO",
      [1] = "FAN_MODE_TEMP",
      [2] = "FAN_MODE_RPM", })
  f.fan_power = ProtoField.bytes("fan.power_mode", "FAN_POWER")
  f.fan_power_mode = ProtoField.int8("fan.fan_power_mode", "power_mode", base.DEC, {
      [0] = "FAN_POWER_MODE_OFF",
      [1] = "FAN_POWER_MODE_ON", })
  f.temp_data = ProtoField.bytes("fan.temp_data", "TEMP_DATA")
  f.temp_data_fan_temp = ProtoField.double("fan.temp_data_fan_temp", "fan_temp")
  f.temp_data_room_temp = ProtoField.double("fan.temp_data_room_temp", "room_temp")
  f.temp_data_fan_temp_valid = ProtoField.int8("fan.temp_data_fan_temp_valid", "fan_temp_valid", base.DEC, {
      [0] = "TEMP_VALIDITY_INVALID",
      [1] = "TEMP_VALIDITY_VALID", })
  f.temp_data_room_temp_valid = ProtoField.int8("fan.temp_data_room_temp_valid", "room_temp_valid", base.DEC, {
      [0] = "TEMP_VALIDITY_INVALID",
      [1] = "TEMP_VALIDITY_VALID", })
  
  -----------------------------------------------------------
  -- Tableau pour stocker les messages possibles et leurs champs respectifs
  local msg = {
    [0] = { name = "MAIN_SHUTDOWN", field = f.main_shutdown, fields = {} }
    [1275] = { name = "FAN_MODE", field = f.fan_mode, fields = {
        [0] = { field = f.fan_mode_mode, t = "int8", size = 1 }, } },
    [1276] = { name = "FAN_POWER", field = f.fan_power, fields = {
        [0] = { field = f.fan_power_mode, t = "int8", size = 1 }, } },
    [1530] = { name = "TEMP_DATA", field = f.temp_data, fields = {
        [0] = { field = f.temp_data_fan_temp, t = "float", size = 4 },
        [1] = { field = f.temp_data_room_temp, t = "float", size = 4 },
        [2] = { field = f.temp_data_fan_temp_valid", t = "int8", size = 1 },
        [3] = { field = f.temp_data_room_temp_valid", t = "int8", size = 1 }} }
  }
  
  -----------------------------------------------------------  
  -- Fonctions de dissection

  local function data(msg_id, buf, tree)
    local m = msg[msg_id]
    local u = m.fields
    local name = m.name
    local tvb
    local cpt = 0
    local subtree
    local bit_fields
    for i = 1, #u do
      bit_fields = u[i].fields
      tvb = buf(cpt, u[i].size)
      if u[i].t == "float" then -- protofield is double to have more precision
        subtree = tree:add(u[i].field, tvb, tvb:float())
      else
        subtree = tree:add(u[i].field, tvb)
      end
      if bit_fields then
        for j = 1, #bit_fields do
          subtree:add(bit_fields[j].field, tvb)
        end
      end
      cpt = cpt + u[i].size
    end
  end

  local function manage_pinfo(msg_id, pinfo)
    pinfo.cols.protocol = "FAN - "..msg[msg_id].name
  end

  -- Fonction de dissection du paquet principal
  function fan.dissector(buf, pinfo, tree)
    local msg_id_info = { idx = 0, size = 4 }
    msg_id = buf(msg_id_info.idx, msg_id_info.size):uint()

    t = tree:add(fan, buf());
    t:set_text("FAN - "..msg[msg_id].name)

    data_size = buf():len() - msg_id_info.size
    data_buf = buf(msg_id_info.size, data_size)
    data_tree = t:add(msg[msg_id].field, data_buf)
    data(msg_id, data_buf, data_tree)

    manage_pinfo(msg_id, pinfo);
  end

  udp_table = DissectorTable.get("udp.port")
  -- Ajout des ports d'ecoute
  for i = 1, #port do
    udp_table:add(port[i], fan)
  end
end
