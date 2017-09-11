function class(super)

    local superType = type(super)
    local cls

    if superType ~= "table" then
        superType = ""
        super = nil
    end

    if super and super.__ctype == 1 then
        -- inherited from native C++ Object
        cls = {__ctype = 2}
        setmetatable(cls, super)
        cls.super    = super
        cls.__index = cls
		
        function cls.new(...)
            local instance = setmetatable({}, cls)
            instance.__ptr = super.new(...)
            instance:ctor(...)
            return instance
        end	
    else
        -- inherited from Lua Object
        if super then
            cls = {}
            setmetatable(cls,super)
            cls.super = super
        else
            cls = {ctor = function() end}
        end

        cls.__index = cls

        function cls.new(...)
            local instance
            if super and super.new then
              instance=setmetatable(super.new(...), cls)
            else
              instance=setmetatable({}, cls)
            end
            instance.class = cls
            instance:ctor(...)
            return instance
        end
    end

    return cls
end


