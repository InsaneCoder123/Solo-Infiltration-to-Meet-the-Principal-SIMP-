mapData = """########################################??????##########,
#                                      #??????#        #,
#                                      #??????#        #,
#                                      #??????#        #,
#                                      #??????#        #,
#                                      #??????#        #,
#                                      #??????#        #,
#                                      ########        #,
#                                                      #,
#                                      ########        #,
#                                      #??????#        #,
#                                      #??????#        #,
#                                      #??????#        #,
#                                      #??????#        #,
#                                      #??????#        #,
########################################??????##########,"""
maxWidth = 0
maxHeight = 0
px = 0
py = 0
ptotal = 0
for i in mapData:
    if i == "\n":
        continue
    if i == ',':
        if py == 0:
            maxWidth = px+1
        py += 1
        px = 0
        continue
    if ptotal % 5 == 0:
        print("")
    if i == ' ':
        type = 0
        state = 0
        ptotal += 1
        print("1{}{}, ".format(state, type), end="")
    if i == '#':
        type = 1
        state = 0
        ptotal += 1
        print("1{}{}, ".format(state, type), end="")
    if i == '?':
        type = 2
        state = 0
        ptotal += 1
        print("1{}{}, ".format(state, type), end="")
    #print("    Position: [{}, {}]       Map Element: [{}]".format(px, py, i))
    px += 1
maxHeight = py
print("\nMax width: {}\nMax height: {}".format(maxWidth, maxHeight))