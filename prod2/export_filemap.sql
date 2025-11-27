SELECT json_agg(t) FROM (
    SELECT id,filepath, (event_id[1] / 30.0)::int AS entry,x,y,z
    FROM protodune3_phbomb_2
    WHERE status='done'
    ORDER BY x ASC, y ASC, z ASC
) t;
