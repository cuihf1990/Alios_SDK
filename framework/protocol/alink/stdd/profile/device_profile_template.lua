local string = require("string")
local table = require("table")
local base = _G
local gw_lib = require("common_lib")
local zigbee_lib = require("zigbee_lib")
local cjson = require("cjson")

version = '1.0'

device_endpoint_profile='[{#foreach($endpoint in $endpoints)"endpoint_id":"0x$endpoint",#if($pEndpoints.containsKey($endpoint))"attr_mapping":[#set($props = $pEndpoints.get($endpoint))#foreach($prop in $props)#set($names = $pNameMap.get($prop.attr))#foreach($name in $names){"alias_name":"$!prop.attr","std_name":"$!name"}#if($velocityCount != $names.size()),#end#end#if($velocityCount != $props.size()),#end#end]#end#if($cEndpoints.containsKey($endpoint)),"cmd_mapping":[#set($cmds = $cEndpoints.get($endpoint))#foreach($cmd in $cmds)#set($names = $cNameMap.get($prop.attr))#foreach($name in $names){"alias_name":"$!cmd.cmd","std_name":"$!name"}#if($velocityCount != $names.size()),#end#end#if($velocityCount != $cmds.size()),#end#end]#end}#end]'

device_attr_set='$!pSet'

device_cmd_set='$!cSet'


#foreach($protocol in $protocols)
$!protocol.script

#end

#if($attrProtocols && $attrProtocols.size() > 0)
function lua_get_custom_attr(dev_id, attr_name, short_model)
	local ret = '-1'
	local device_attr_name, endpoint_id
	local endpoint_id, attr_set = gw_lib.c_get_attr_mapping_set(tonumber(short_model), attr_name)
	if(endpoint_id == nil or attr_set == nil) then
		return ret
    end

	base.print("lua_get_custom_attr, attr_name = "..attr_name)
	local attr_tab = cjson.decode(attr_set)
    if(attr_tab == nil) then
    	base.print("invalid attribute set:"..attr_set)
    	return ret
	end

    for i = 1, #attr_tab do
        zigbee_attr = attr_tab[i]

#foreach($protocol in $attrProtocols)
#set($detail = $detailMap.get($protocol.id))
#set($name = $detail.get("attr_name"))
#if($velocityCount == 1)
#set($prefix = "")
#else
#set($prefix = "else")
#end
        ${prefix}if(zigbee_attr == '$!name' and __get_${name}_attr ~= nil) then
            ret = __get_${name}_attr(dev_id, endpoint_id)
#end
        end
    end

    return ret
end

#end


#if(($rwProtocols && $rwProtocols.size() > 0) || ($exceProtocols && $exceProtocols.size() > 0))
function lua_set_custom_attr(dev_id, attr_name, attr_value, short_model)
	local ret = '-1'
	local device_attr_name, endpoint_id
	local endpoint_id, attr_set = gw_lib.c_get_attr_mapping_set(tonumber(short_model), attr_name)
	if(endpoint_id == nil or attr_set == nil) then
		return ret
	end

	base.print("lua_set_custom_attr, attr_name = "..attr_name..", attr_value = "..attr_value)

    local attr_tab = cjson.decode(attr_set)
    if (attr_tab == nil) then
        base.print("invalid attribute set:"..attr_set)
        return ret
    end
#if($exceProtocols && $exceProtocols.size() > 0)
    for i = 1, #attr_tab do
        zigbee_attr = attr_tab[i]

#foreach($protocol in $exceProtocols)
#set($detail = $detailMap.get($protocol.id))
#set($name = $detail.get("cmd_name"))
#if($velocityCount == 1)
#set($prefix = "")
#else
#set($prefix = "else")
#end
        ${prefix}if(zigbee_attr == '$name' and __set_${name}_attr ~= nil) then
            ret = __set_${name}_attr(dev_id, endpoint_id, attr_name, attr_value)
#end
        end
    end
#end

#if($rwProtocols && $rwProtocols.size() > 0)
    for i = 1, #attr_tab do
        zigbee_attr = attr_tab[i]

#foreach($protocol in $rwProtocols)
#set($detail = $detailMap.get($protocol.id))
#set($name = $detail.get("attr_name"))
#if($velocityCount == 1)
#set($prefix = "")
#else
#set($prefix = "else")
#end
        ${prefix}if(zigbee_attr == '$name' and __set_${name}_attr ~= nil) then
            ret = __set_${name}_attr(dev_id, endpoint_id, attr_name, attr_value)
#end
        end
    end
#end

    return ret
end
#end

#if($attrProtocols && $attrProtocols.size() > 0)
function lua_report_custom_attr(dev_id, endpoint_id, attr_name, attr_value, short_model)
	base.print("lua_report_custom_attr, endpoint_id = "..endpoint_id..", attr_name = "..attr_name)
	local ret = '-1'
	local cloud_attr_name
	local attr_set = gw_lib.c_get_alink_attr_set(tonumber(short_model), tonumber(endpoint_id), attr_name)
	if(attr_set == nil) then
		return ret
	end

	local attr_tab = cjson.decode(attr_set)
	if(attr_tab == nil) then
		return ret
	end

#foreach($protocol in $attrProtocols)
#set($detail = $detailMap.get($protocol.id))
#set($name = $detail.get("attr_name"))
#if($velocityCount == 1)
#set($prefix = "")
#else
#set($prefix = "else")
#end
    ${prefix}if(attr_name == '$name' and __report_${name}_attr ~= nil) then
        ret = __report_${name}_attr(dev_id, attr_tab, attr_value)
#end
    end

	return ret
end
#end


#if($execProtocols && $execProtocols.size() > 0)
function lua_exec_custom_cmd(dev_id, cmd_name, cmd_args, short_model)
	local ret = '-1'
	local device_cmd_name, endpoint_id
	local endpoint_id, zigbee_cmd = gw_lib.c_get_cmd_mapping_name(tonumber(short_model), cmd_name)
	if(endpoint_id == nil or zigbee_cmd == nil) then
		return ret
	end

    base.print("lua_exec_custom_cmd, cmd_name = "..cmd_name..", cmd_args = "..cmd_args)


#foreach($protocol in $execProtocols)
#set($detail = $detailMap.get($protocol.id))
#set($name = $detail.get("cmd_name"))
#if($velocityCount == 1)
#set($prefix = "")
#else
#set($prefix = "else")
#end
	${prefix}if(zigbee_cmd == '$name' and __exec_$name_cmd ~= nil) then
		ret = __exec_$name_cmd(dev_id, endpoint_id, zigbee_cmd, cmd_args)
#end
	end
    return ret
end
#end


#if($reportProtocols && $reportProtocols.size() > 0)
function lua_report_custom_event(dev_id, endpoint_id, event_name, event_args, short_model)
	base.print("lua_report_custom_event, endpoint_id = "..endpoint_id..", event_name = "..event_name..", event_args = "..event_args)
	local ret = '-1';
	local attr_set = gw_lib.c_get_alink_attr_set(tonumber(short_model), tonumber(endpoint_id), event_name)
	if(attr_set == nil) then
		return ret
	end

	local attr_tab = cjson.decode(attr_set)
	if(attr_tab == nil) then
		return ret
	end

#foreach($protocol in $reportProtocols)
#set($detail = $detailMap.get($protocol.id))
#set($name = $detail.get("cmd_name"))
#if($velocityCount == 1)
#set($prefix = "")
#else
#set($prefix = "else")
#end
    ${prefix}if(event_name == '${name}' and __report_${name}_event ~= nil) then
        do return __report_${name}_event(dev_id, attr_tab, event_args) end
#end
    end

	return ret
end
#end

