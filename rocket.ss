[config]
delta_t 1

[all_states]
enableEngine:  verticalVelocity = verticalVelocity + 10
!enableEngine: verticalVelocity = verticalVelocity - 10
verticalVelocity < 0 and deployParachute: verticalVelocity = -3
true: altitude = altitude + verticalVelocity
altitude <= 0 {
    verticalVelocity = 0
    altitude = 0
}
G > 100 and altitude == 0 {
    @assert popNosecone
    @assert deployParachute
    @assert S == 4
}
G == 102 {
    @stop
}

[Ascent]
true: @assert enableEngine == (T < 5)

[Descent]
true {
    @assert !enableEngine
    @assert ventOpen == (5 <= T < 10)
}

[Touchdown]
true: @assert !enableEngine
